/*
* image_recv.c
 *
 *  Created on: 6.10.2014
 *      Author: Patrick Kuncke 2094460
 *				Philipp Kloth 2081738
 *  Version 1.0
 */

#include "raspberryView.h"
 
void printInfos(unsigned char *, int);
 
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