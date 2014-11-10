/*
* image_recv.c
 *
 *  Created on: 6.10.2014
 *      Author: Patrick Kuncke 2094460
 *				Philipp Kloth 2081738
 *  Version 1.0
 */

//#define DEBUG_MSG
 
#include "raspberryView.h"
 
void printInfos(unsigned char *, int);

struct buffer{
    unsigned char *buf;
    int size;
    int isUsed;
};

void updateGUI(struct buffer *buffer, int index, GError *error, gpointer data);
 
void recv_func(gpointer data)
{
	/*
		Gui Initialisierung
		
	*/

#ifdef DEBUG_MSG
    printf("go go go...\n");
#endif    
	int sock_raw;
	FILE *logfile;
	int index=0;
    int saddr_size , data_size;
    struct sockaddr saddr;
	//FILE *fp;
    struct buffer bf1;
    //bf1->buffer = (unsigned char *)malloc(IMG_MAX);
    struct buffer bf2;
//    bf2->buffer = malloc(IMG_MAX);
//    struct buffer *bf3;
//    bf3->buffer = (unsigned char *)malloc(IMG_MAX);
    struct buffer * buffer;
	long seconds, useconds;
#ifdef DEBUG_MSG
    printf("go go go...\n");
#endif 
    
	GError    *error = NULL;

    
	unsigned char *rcbuffer = (unsigned char *)malloc(MTU); //1514
    //bf1->buffer = (unsigned char *)malloc(IMG_MAX);
    
    unsigned char *buffer1 = (unsigned char *)malloc(IMG_MAX); //Its Big! 30000
    
    unsigned char *buffer2 = (unsigned char *)malloc(IMG_MAX);
	
    bf1.buf = buffer1;
    bf2.buf = buffer2;
    bf1.isUsed = 0;
    bf2.isUsed = 0;
    buffer = &bf1;
    
    
        
     
    logfile=fopen("log.txt","w");
    if(logfile==NULL) printf("Unable to create file.");
#ifdef DEBUG_MSG
    printf("Starting...\n");
#endif
    //Create a raw socket that shall sniff
    sock_raw = socket(AF_PACKET , SOCK_RAW , htons (ETH_P_ALL));
	
    if(sock_raw < 0)
    {
#ifdef DEBUG_MSG
        printf("Socket Error\n");
#endif
    }else
	{
#ifdef DEBUG_MSG
		printf("Socket erstellt\n");
#endif
	}
	
	inet_adress = inet_addr(FRONTCAM);
	
	//Zeitmessung
	struct timeval start, end; //Definierung der Variablen
	gettimeofday(&start, 0); //CPU-Zeit zu Beginn des Programmes

    while( TRUE )
    {
		
		
		saddr_size = sizeof saddr;
        //Receive a packet
        data_size = recvfrom(sock_raw , rcbuffer , MTU , 0 , &saddr ,(socklen_t *) &saddr_size);
		//WICHTIG! Wegen Ethernet Header den wir mit Sniffen!
		struct iphdr *iph = (struct iphdr*)(rcbuffer + sizeof(struct ethhdr));
		
        if(data_size <0 )
        {
#ifdef DEBUG_MSG
            printf("Recvfrom error , failed to get packets\n");
#endif
        }else{
#ifdef DEBUG_MSG
			struct sockaddr_in source;	
			memset(&source, 0, sizeof(source));
			source.sin_addr.s_addr = iph->saddr;
			printf("Packet empfangen von %s\n",inet_ntoa(source.sin_addr));
#endif
		}
        //Now process the packet
		//Get the IP Header part of this packet
	
		if(iph->saddr == inet_adress)
		{	
			#ifdef DEBUG_MSG
			printf("IP Offset   : %d\n",ntohs(iph->frag_off));
			#endif
			
			//Einziges oder letztes Paket
			if((ntohs(iph->frag_off) & 0x1fff) == 0){
				#ifdef DEBUG_MSG
				printf("Erste 3 Bits des IP Offset sind 000 \n");
				printf("IP Offset Flags: %d\n",(ntohs(iph->frag_off) & 0x1fff));
				
				printf("IF IP Packet Size: %d\n", data_size);
				#endif
				
				memcpy(buffer->buf +index, rcbuffer+IP_HEADER+ETHERNET_HEADER , data_size-IP_HEADER-ETHERNET_HEADER);
				index += (data_size-IP_HEADER-ETHERNET_HEADER);
				#ifdef DEBUG_MSG
				printf("Bildgroeße (Index): %d\n", index);
				#endif
				// //GUI informieren
	
				buffer->size = index;
                
                if(!buffer->isUsed){
				    buffer->isUsed = 1;
                    int sizeT = buffer->size;
                    updateGUI(buffer, sizeT, error, data);
                    
                    if(buffer==&bf1){
						//printf("Buffer 1 wurde verwendet\n");
                        buffer = &bf2;
                    } else {
						//printf("Buffer 2 wurde verwendet\n");
                        buffer = &bf1;
                    }
                
                }
				
				index=0;
				#ifdef DEBUG_MSG
				printf("Index zurueck gesetzt\n");
				#endif
				
				//--------------------------------
				//Auswertung der Zeitmessung
				gettimeofday(&end, 0); //CPU-Zeit am Ende des Programmes
				
				printf("Start: %lu:%lu\n",start.tv_sec, start.tv_usec);
				printf("Ende: %lu:%lu\n",end.tv_sec, end.tv_usec);
				
				seconds = end.tv_sec - start.tv_sec;
				useconds = end.tv_usec - start.tv_usec;
				if(useconds < 0) {
					useconds += 1000000;
					seconds--;
				}   

				printf("Dauer des Programms (Empfang + weitergabe an GUI):        %lu sec %lu usec\n\n", seconds, useconds);
				gettimeofday(&start, 0); //CPU-Zeit zu Beginn des Programmes (neuer Durchlauf)
				//----------------------------------
				
				
			}
			//Es folgen weitere Pakete
			else{
				#ifdef DEBUG_MSG
				printf("Erste 3 Bits des IP Offset sind NICHT 000 \n");
				printf("IP Offset Flags: %d\n",(ntohs(iph->frag_off) & 0x1fff));
				
				printf("ELSE IP Packet Size: %d\n", data_size);
				
				//Kopieren von Paket - IP Header und Ethernet Header von rcbuffer in buffer
				printf("Index vorm Memcpy: %d\n", index);
				#endif
				memcpy(buffer->buf+index, rcbuffer+IP_HEADER+ETHERNET_HEADER , MTU-IP_HEADER-ETHERNET_HEADER);
				index += (MTU-IP_HEADER-ETHERNET_HEADER);
				
				#ifdef DEBUG_MSG
				printf("Index nach Memcpy: %d\n", index);
				#endif
			}
			
			
			
#ifdef DEBUG_MSG
				//printInfos(buffer, data_size);
#endif		
		}
  
    }
	
	close(sock_raw);
    printf("Finished");
}

void updateGUI(struct buffer *buffer, int index, GError *error, gpointer data){
	GdkPixbuf * pixbuf;	
	GdkPixbufLoader *loader;
				
	loader = gdk_pixbuf_loader_new ();
	gdk_pixbuf_loader_write (loader, (guint8 *)buffer->buf,(gsize)index, NULL);
	pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
	
	if(pixbuf!=NULL){
		
		g_object_ref(pixbuf);
	
		//Unref Loader
		gdk_pixbuf_loader_close(loader, &error );
		g_object_unref(loader);
	
		
		//Gute Qualität - Gute Geschwindigkeit
		//gdk_pixbuf_scale_simple(pixbuf, 320, 240, GDK_INTERP_BILINEAR  );
		//Beste Geschwindigkeit
		//gdk_pixbuf_scale_simple(pixbuf, 320, 240, GDK_INTERP_NEAREST);
	
		//betrete kritische Zone
		gdk_threads_enter();
	
		gtk_image_set_from_pixbuf(GTK_IMAGE(data), pixbuf );
	
		//verlasse kritische Zone
		gdk_threads_leave();
	
		g_object_unref(pixbuf);
	
		//gdk_threads_add_idle ((GSourceFunc) update_function, pixbuf);
	}
    buffer->isUsed = 0;
}


void printInfos(unsigned char *Buffer , int Size){
	
	unsigned short iphdrlen;
    
	struct sockaddr_in source,dest;	
    struct iphdr *iph = (struct iphdr *)Buffer;
    iphdrlen =iph->ihl*4;
	
	memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
     
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;
	
	struct udphdr *udph = (struct udphdr*)(Buffer + iphdrlen);
	
	printf("###########################################################\n");
	printf("   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)iph->ihl,((unsigned int)(iph->ihl))*4);
	printf("   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(iph->tot_len));
	printf("   |-Source IP         : %s\n",inet_ntoa(source.sin_addr));
    printf("   |-Destination IP    : %s\n",inet_ntoa(dest.sin_addr));
	printf("   |-Protocol          : %d\n",(unsigned int)iph->protocol);
	printf("   |-Source Port       : %d\n" , ntohs(udph->source));
	printf("   |-Destination Port  : %d\n" , ntohs(udph->dest));
	printf("   |-UDP Length        : %d\n" , ntohs(udph->len));
	printf("###########################################################\n\n");
	
}
