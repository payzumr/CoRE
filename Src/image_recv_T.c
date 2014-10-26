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
 
void recv_func(gpointer data)
{
	/*
		Gui Initialisierung
		
	*/
	int sock_raw;
	FILE *logfile;
	int udp=0;
    int saddr_size , data_size;
    struct sockaddr saddr;
	FILE *fp;
     
    unsigned char *buffer = (unsigned char *)malloc(IP_HEADER_MAX); //Its Big!
     
    logfile=fopen("log.txt","w");
    if(logfile==NULL) printf("Unable to create file.");
#ifdef DEBUG_MSG
    printf("Starting...\n");
#endif
    //Create a raw socket that shall sniff
    sock_raw = socket(AF_INET , SOCK_RAW , IPPROTO_UDP);
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

    while( TRUE )
    {
        
		saddr_size = sizeof saddr;
        //Receive a packet
        data_size = recvfrom(sock_raw , buffer , IP_HEADER_MAX , 0 , &saddr ,(socklen_t *) &saddr_size);
        if(data_size <0 )
        {
#ifdef DEBUG_MSG
            printf("Recvfrom error , failed to get packets\n");
#endif
        }else{
			printf("Packet empfangen\n");
		}
		//printf("Recvfrom: Packet empfangen\n");
        //Now process the packet
		//Get the IP Header part of this packet
		
		
		struct iphdr *iph = (struct iphdr*)buffer;
		
		if((ntohs(iph->frag_off) & 0x3fff) == 0){
			printf("Erste 3 Bits des IP Offset sind 000 \n");
		}else{
			printf("Erste 3 Bits des IP Offset sind NICHT 000 \n");
		}
		
		
		
		//fprintf("IP Offset   : %d\n",(unsigned int)iphdr->ip_off);
		
		if(iph->saddr == inet_adress)
		{
			if(iph->protocol==UDP_PROTO)
			{
				++udp;
#ifdef DEBUG_MSG
				printf("UDP packet received   Count: %d\n", udp);
#endif
		
			}
			else
			{
#ifdef DEBUG_MSG
				printf("Other Protocol\n");
#endif
				break;
			}
		
			unsigned short iphdrlen;
		
    		iphdrlen = iph->ihl*4;
     
    		struct udphdr *udph = (struct udphdr*)(buffer + iphdrlen);
		
			if(4950 == ntohs(udph->source))
			{
				fp = fopen("output.jpg", "w");
				fwrite(buffer+28, data_size- sizeof udph - iph->ihl * 4, 1, fp);
				
				//GUI Aufruf
				gdk_threads_enter();
				gtk_image_set_from_file(GTK_IMAGE( data ),"output.jpg");
				gdk_threads_leave();
				
				fclose(fp);
#ifdef DEBUG_MSG
				printInfos(buffer, data_size);
#endif	
			}
		}
  
    }
	
	close(sock_raw);
    printf("Finished");
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