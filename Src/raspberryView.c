#include "raspberryView.h"

#define THREADE(returnVal, text) if((returnVal) != 0) { errno = returnVal; perror(text);}

  pthread_t dummy;

  GtkWidget *win;
  GtkWidget *image;
  GtkWidget *align;

  GtkWidget *button1;
  GtkWidget *button2;

  GtkWidget *hbox;
  GtkWidget *vbox;
  
  GtkLabel *label;
  GtkHSeparator *sep;
  
  GThread   *thread;
  GError    *error = NULL;
  
  
static gpointer thread_func( gpointer data )
{
	/*
		Gui Initialisierung
		
	*/
	int sock_raw;
	FILE *logfile;
	int udp=0,others=0,i,j;
	struct sockaddr_in source,dest;
    int saddr_size , data_size;
    struct sockaddr saddr;
    struct in_addr in;
	FILE *fp;
     
    unsigned char *buffer = (unsigned char *)malloc(IP_HEADER_MAX); //Its Big!
     
    logfile=fopen("log.txt","w");
    if(logfile==NULL) printf("Unable to create file.");
    printf("Starting...\n");
    //Create a raw socket that shall sniff
    sock_raw = socket(AF_INET , SOCK_RAW , IPPROTO_UDP);
    if(sock_raw < 0)
    {
        printf("Socket Error\n");
    }else{
		printf("Socket erstellt\n");
	}
	
	//Threads starten 1 gui 2 while(1)
	int num = 0;


    while( TRUE )
    {
        
		saddr_size = sizeof saddr;
        //Receive a packet
        data_size = recvfrom(sock_raw , buffer , IP_HEADER_MAX , 0 , &saddr , &saddr_size);
        if(data_size <0 )
        {
            printf("Recvfrom error , failed to get packets\n");
        }
		//printf("Recvfrom: Packet empfangen\n");
        //Now process the packet
		//Get the IP Header part of this packet
		struct iphdr *iph = (struct iphdr*)buffer;
		
		if(iph->protocol==UDP_PROTO){
			++udp;
			printf("UDP packet received   Count: %d\n", udp);
		
		}else{
			printf("Other Protocol\n");
			break;
		}
		
		unsigned short iphdrlen;
		
    	iphdrlen = iph->ihl*4;
     
    	struct udphdr *udph = (struct udphdr*)(buffer + iphdrlen);
		
		if(4950 == ntohs(udph->source)){
			num = (num + 1) % 2;
			if(num == 0){
				fp = fopen("output.jpg", "w");
				fwrite(buffer+28, data_size- sizeof udph - iph->ihl * 4, 1, fp);
		
		
				//GUI Aufruf

				gdk_threads_enter();
				gtk_image_set_from_file(GTK_IMAGE( data ),"output.jpg");
				gdk_threads_leave();
				
				
				}else{
					fp = fopen("output1.jpg", "w");
					fwrite(buffer+28, data_size- sizeof udph - iph->ihl * 4, 1, fp);
		
		
					//GUI Aufruf

					gdk_threads_enter();
					gtk_image_set_from_file(GTK_IMAGE( data ),"output1.jpg");
					gdk_threads_leave();
				
				}
				
		
			fclose(fp);
		
			//printInfos(buffer, data_size);
		
		}
  
    }
	
	close(sock_raw);
    printf("Finished");

    return( NULL );
} 

		    
static void handleButton1(GtkWidget *widget,GtkImage  *image){
  g_print("Button 1 wurde geklickt \n");
  gtk_label_set_text(label,"Auto Kamera vorne \n");
  gtk_image_set_from_file(image,"output.jpg");
}


static void handleButton2(GtkWidget *widget,GtkImage  *image){
  g_print("Button 2 wurde geklickt \n");
  gtk_label_set_text(label,"Auto Kamera hinten \n");
  gtk_image_set_from_file(image,"output1.jpg");
}

static void finishApplication(){
  g_print("Fnished now \n");
  gtk_main_quit();
  
}

static void initWindow(){

  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(win),"Ein Auto");
  gtk_window_set_default_size(GTK_WINDOW(win),320,240);
  gtk_window_set_resizable(GTK_WINDOW(win),FALSE);
  //gtk_window_fullscreen(GTK_WINDOW(win));
  //gtk_window_set_position(GTK_WINDOW(win),GTK_WIN_POS_CENTER);
  
}

static void initWidgets(){
 
  image = g_object_new(GTK_TYPE_IMAGE,"file","output.jpg",NULL);
  align = gtk_alignment_new(1,0,0,0);
  vbox = gtk_vbox_new(FALSE,3);
  hbox = gtk_hbox_new(FALSE,3);
  sep = g_object_new(GTK_TYPE_HSEPARATOR,NULL);
  
  label = g_object_new(GTK_TYPE_LABEL,"label","Auto Kamera!",NULL);
  button1 = gtk_button_new_with_label("next");
  button2 = gtk_button_new_with_label("prev");
  
  gtk_widget_set_size_request(button1,80,35);
  gtk_widget_set_size_request(button2,80,35);  

}
	
int main(int argc, char *argv[]){
	
	/* Secure glib */
    if( ! g_thread_supported() )
        g_thread_init( NULL );
		
	/* Secure gtk */
    gdk_threads_init();

    /* Obtain gtk's global lock */
    gdk_threads_enter();

	// if((pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) !=0)
   // {
      // fprintf(stderr,"Fehler bei pthread_setcancelstate.......\n");
      // exit(0);
  // }
	
	// THREADE(pthread_create( &dummy, NULL, recvThread, NULL), "Error: create Thread");
	
	gtk_init(&argc, &argv);
  
  initWindow();
  initWidgets(); 
  
  g_signal_connect(GTK_OBJECT(button1),"clicked",G_CALLBACK(handleButton1),GTK_IMAGE( image ));
  g_signal_connect(GTK_OBJECT(button2),"clicked",G_CALLBACK(handleButton2),GTK_IMAGE( image ));
  
  //g_signal_connect(GTK_OBJECT(win),"delet-event",G_CALLBACK(destroyWindow),NULL);
  g_signal_connect(GTK_OBJECT(win),"destroy",G_CALLBACK(finishApplication),NULL);
  
  gtk_container_add(GTK_CONTAINER(align),hbox);
  
  gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(label),FALSE,FALSE,1);
  
  gtk_box_pack_start(GTK_BOX(hbox),button2,FALSE,FALSE,1);
  gtk_box_pack_start(GTK_BOX(hbox),button1,FALSE,FALSE,1);
  
  gtk_box_pack_start(GTK_BOX(vbox),image,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(sep),FALSE,TRUE,5);
  gtk_box_pack_start(GTK_BOX(vbox),align,FALSE,FALSE,0);
  
  gtk_container_add(GTK_CONTAINER(win),vbox);
  
  //timer = gtk_timeout_add(40,(GtkFunction)updateTimer,GTK_IMAGE( image ));
  
  /* Create new thread */
    thread = g_thread_create( thread_func, (gpointer)image, FALSE, &error );
							  
	if( ! thread )
    {
        g_print( "Error: %s\n", error->message );
        return( -1 );
    }
  
  gtk_widget_show_all(GTK_WIDGET(win));
  
  gtk_main();
  
  /* Release gtk's global lock */
  gdk_threads_leave();

  g_print(":::Ende der Applikation::: \n");
	
	//THREADE(pthread_join( dummy, NULL ), "Error: join Thread");
	
	
	return 0;
}