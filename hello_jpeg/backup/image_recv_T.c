/*
* image_recv.c
 *
 *  Created on: 6.10.2014
 *      Author: Patrick Kuncke 2094460
 *				Philipp Kloth 2081738
 *  Version 1.0
 */
#define DEBUG_MSG
#define IP_MF 0x2000            /* more fragments flag */
 
#include "raspberryView.h"
 
int mouseClick = 0;
void CallBackFunc(int event, int x, int y, int flags, void*userdata)
{
    if(event == CV_EVENT_LBUTTONDOWN)
    {
        if(mouseClick)
        {
            mouseClick = 0;
        }
        else
        {
            mouseClick = 1;
        }
    }
}
 
void recv_func()
{
	/* ------------------ Variablen --------------------*/
    int erster_frame = 1;	        
    int sock_raw;
    FILE *logfile;
    int index=0;
    int saddr_size , data_size;
    struct sockaddr saddr;
    long seconds, useconds;
	
    /*---------------------Buffer malloc -----------------*/	  
    unsigned char *rcbuffer = (unsigned char *)malloc(MTU); //1514
    unsigned char *buffer = (unsigned char *)malloc(200000); //Its Big! 30000
    unsigned char * outBuffer = (char *)malloc(500000);
    
    int s;
    OPENMAX_JPEG_DECODER *pDecoder;
    printf("Start\n");
    bcm_host_init();
    
    s = setupOpenMaxJpegDecoder(&pDecoder);
	
    printf("Decoder initialized\n");
    //Create a raw socket that shall sniff
    sock_raw = socket(AF_PACKET , SOCK_RAW , htons (ETH_P_ALL));
	
#ifdef DEBUG_MSG 	
    if(sock_raw < 0)
    {
        printf("Socket Error\n");
    }
    else
    {
        printf("Socket erstellt\n");
    }
#endif
	
    inet_adress = inet_addr(TEMPCAM);
	
    //Zeitmessung
#ifdef DEBUG_MSG
    struct timeval start, end, end2; //Definierung der Variablen
#endif

    /*---------------------------------------------*/
    /*--------------- Empfangsdauerschleife -----------*/
    while( 1 )
    {
	saddr_size = sizeof saddr;
        //Receive a packet
        data_size = recvfrom(sock_raw , rcbuffer , MTU , 0 , &saddr ,(socklen_t *) &saddr_size);
        //WICHTIG! Wegen Ethernet Header den wir mit Sniffen!
	struct iphdr *iph = (struct iphdr*)(rcbuffer + sizeof(struct ethhdr));
		
#ifdef DEBUG_MSG		
        if(data_size <0 )
        {
            printf("Recvfrom error , failed to get packets\n");
        }
        else
        {
            
	}
#endif
	/*--------------------Prüfung ob Paket von einer wichtigen IP kommt ----- */
	if(iph->saddr == inet_adress)
	{	
#ifdef DEBUG_MSG
			struct sockaddr_in source;	
            memset(&source, 0, sizeof(source));
            source.sin_addr.s_addr = iph->saddr;
            printf("Packet empfangen von %s\n",inet_ntoa(source.sin_addr));
            printf("IP Offset   : %d\n",ntohs(iph->frag_off));
			printf("More Fragments: %d\n",(ntohs(iph->frag_off) & IP_MF));
#endif
	
	//Einziges oder letztes Paket
			
            //if((ntohs(iph->frag_off) & 0x1fff) == 0)
			if((ntohs(iph->frag_off) & IP_MF) == 0)
            {
#ifdef DEBUG_MSG
		printf("Erste 3 Bits des IP Offset sind 000 \n");
		
		printf("IP Offset Flags: %d\n",(ntohs(iph->frag_off) & 0x1fff));
		printf("IF IP Packet Size: %d\n", data_size);
#endif
				
		memcpy(buffer +index, rcbuffer+IP_HEADER+ETHERNET_HEADER , data_size-IP_HEADER-ETHERNET_HEADER);
		index += (data_size-IP_HEADER-ETHERNET_HEADER);
		
		logfile = fopen("temp_img.jpg","w+");
		
		fprintf(buffer, 1, index,logfile);
		
		fclose(logfile);
				
#ifdef DEBUG_MSG
		printf("Bildgroeße (Index): %d\n", index);
				
				
		//--------------------------------
		//Auswertung der Zeitmessung
		gettimeofday(&end2, 0); //CPU-Zeit am Ende des Bildempfangens
				
		seconds = end2.tv_sec - start.tv_sec;
		useconds = end2.tv_usec - start.tv_usec;
		if(useconds < 0) 
                {
                    useconds += 1000000;
                    seconds--;
		}   

                printf("Dauer des Bildempfangs:        %lu sec %lu usec und groesse %d\n\n", seconds, useconds, index);

		//Zeitmessung
		long seconds, useconds;
				
		struct timeval startupdate, endupdate; //Definierung der Variablen
		gettimeofday(&startupdate, 0); //CPU-Zeit zu Beginn des Programmes
#endif
		/*-------- PI JPEG Hardwaredecode---------------*/
				
		unsigned int s;
		s = decodeImage(pDecoder, buffer, (size_t)index, outBuffer );
		printf("Groeße nach Decode: %d\n");
				
#ifdef DEBUG_MSG
                gettimeofday(&endupdate, 0); //CPU-Zeit am Ende des Bildempfangens
		
                seconds = endupdate.tv_sec - startupdate.tv_sec;
                useconds = endupdate.tv_usec - startupdate.tv_usec;
                if(useconds < 0) 
                {
                    useconds += 1000000;
                    seconds--;
                
                }   
                printf("Dauer des DECODE:        %lu sec %lu usec\n\n", seconds, useconds);
				  
                gettimeofday(&startupdate, 0); //CPU-Zeit zu Beginn des Programmes
#endif
				  /*-----------------------------------------------*/
                IplImage* image;
		image = cvCreateImageHeader(cvSize(320, 240), IPL_DEPTH_8U, 4); // Bild mit 320*240
		image->imageData = outBuffer;

		cvShowImage("Simulator", image);
		if ((cvWaitKey(5) & 255) == 27)	break;
#ifdef DEBUG_MSG
                gettimeofday(&endupdate, 0); //CPU-Zeit am Ende des Bildempfangens
			
		seconds = endupdate.tv_sec - startupdate.tv_sec;
		useconds = endupdate.tv_usec - startupdate.tv_usec;
		if(useconds < 0) 
                {
                    useconds += 1000000;
                    seconds--;
		}   
		printf("Dauer des GUI Update:        %lu sec %lu usec\n\n", seconds, useconds);
				
		//Auswertung der Zeitmessung
#endif
		//----------------------------------
				
		index=0;
#ifdef DEBUG_MSG
                printf("Index zurueck gesetzt\n");
		gettimeofday(&end, 0); //CPU-Zeit am Ende des Programmes
				
		printf("Start: %lu:%lu\n",start.tv_sec, start.tv_usec);
		printf("Ende: %lu:%lu\n",end.tv_sec, end.tv_usec);

                seconds = end.tv_sec - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		if(useconds < 0) 
                {
                    useconds += 1000000;
                    seconds--;
		}   

		printf("Dauer des Programms (Empfang + weitergabe an GUI):        %lu sec %lu usec\n\n", seconds, useconds);
                gettimeofday(&start, 0); //CPU-Zeit zu Beginn des Programmes (neuer Durchlauf)
#endif
                erster_frame = 1;
		//----------------------------------
		(mouseClick == 0) ? (inet_adress = inet_addr(FRONTCAM)) : (inet_adress = inet_addr(BACKCAM));	
				
            }
			//Es folgen weitere Pakete
            else if(iph->saddr == inet_adress)
            {
                
                if(erster_frame)
                {
#ifdef DEBUG_MSG
                    gettimeofday(&start, 0); //CPU-Zeit zu Beginn des Programmes
#endif
                    erster_frame = 0;
                }
#ifdef DEBUG_MSG
                printf("Erste 3 Bits des IP Offset sind NICHT 000 \n");
                printf("IP Offset Flags: %d\n",(ntohs(iph->frag_off) & 0x1fff));
				
                printf("ELSE IP Packet Size: %d\n", data_size);
				
                //Kopieren von Paket - IP Header und Ethernet Header von rcbuffer in buffer
                printf("Index vorm Memcpy: %d\n", index);
#endif
                memcpy(buffer+index, rcbuffer+IP_HEADER+ETHERNET_HEADER , MTU-IP_HEADER-ETHERNET_HEADER);
                index += (MTU-IP_HEADER-ETHERNET_HEADER);
				
#ifdef DEBUG_MSG
                printf("Index nach Memcpy: %d\n", index);
#endif
            }	
        }
    }
	
	close(sock_raw);
        printf("Finished");
}

int main(int argc, char *argv[]) 
{
    cvNamedWindow("Simulator", CV_WINDOW_AUTOSIZE);
    cvSetMouseCallback("Simulator", CallBackFunc, NULL);
    IplImage *startImage;
    startImage  = cvLoadImage("start.jpg", CV_LOAD_IMAGE_COLOR);
    cvShowImage("Simulator", startImage);
    cvWaitKey(20);
    printf("wait durch...\n");
    
    
    recv_func();
    cvReleaseImage(&startImage);
    
    return 0;     //nach terminierung aller Threads terminiert main
}
