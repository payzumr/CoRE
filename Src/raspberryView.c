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
	recv_func(data);
} 

		    
static void handleButton1(GtkWidget *widget,GtkImage  *image){
#ifdef DEBUG_MSG
  	g_print("Button 1 wurde geklickt \n");
#endif
  	gtk_label_set_text(label,"Auto Kamera vorne \n");
  	gtk_image_set_from_file(image,"output.jpg");
}


static void handleButton2(GtkWidget *widget,GtkImage  *image){
#ifdef DEBUG_MSG
  	g_print("Button 2 wurde geklickt \n");
#endif
  	gtk_label_set_text(label,"Auto Kamera hinten \n");
  	gtk_image_set_from_file(image,"output1.jpg");
}

static void finishApplication(){
#ifdef DEBUG_MSG
  	g_print("Fnished now \n");
#endif
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

		
	gtk_init(&argc, &argv);
  
  	initWindow();
  	initWidgets(); 
  
  	g_signal_connect(GTK_OBJECT(button1),"clicked",G_CALLBACK(handleButton1),GTK_IMAGE( image ));
  	g_signal_connect(GTK_OBJECT(button2),"clicked",G_CALLBACK(handleButton2),GTK_IMAGE( image ));
  
 	g_signal_connect(GTK_OBJECT(win),"destroy",G_CALLBACK(finishApplication),NULL);
  
  	gtk_container_add(GTK_CONTAINER(align),hbox);
      
  	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(label),FALSE,FALSE,1);
  
  	gtk_box_pack_start(GTK_BOX(hbox),button2,FALSE,FALSE,1);
  	gtk_box_pack_start(GTK_BOX(hbox),button1,FALSE,FALSE,1);
  
  	gtk_box_pack_start(GTK_BOX(vbox),image,FALSE,FALSE,0);
 	gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(sep),FALSE,TRUE,5);
  	gtk_box_pack_start(GTK_BOX(vbox),align,FALSE,FALSE,0);
  
  	gtk_container_add(GTK_CONTAINER(win),vbox);
  
  	/* Create new thread */
    thread = g_thread_create( thread_func, (gpointer)image, FALSE, &error );
							  
	if( ! thread )
    {
#ifdef DEBUG_MSG
        g_print( "Error: %s\n", error->message );
#endif
        return( -1 );
    }
  
  	gtk_widget_show_all(GTK_WIDGET(win));
  
  	gtk_main();
  
  	/* Release gtk's global lock */
  	gdk_threads_leave();

  	g_print(":::Ende der Applikation::: \n");
	
	
	return 0;
}