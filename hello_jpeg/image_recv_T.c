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
#ifdef DEBUG_MSG
int pakete=0;
CvScalar color;
CvFont font;
char text[255];
int pictures=0;
#endif
int firstPacket=1;


unsigned char camMacfront[6] = {0xd4, 0xbe, 0xd9, 0x69, 0xca, 0xb5};
unsigned char camMacback[6] = {0xd4, 0xbe, 0xd9, 0x69, 0xca, 0xb6};
unsigned char *camMac;
unsigned char dataMac[6] = {0xd4, 0xbe, 0xd9, 0x69, 0xca, 0xb5};



int checkMacAddr(unsigned char * src, unsigned char * value);

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
	
	#ifdef DEBUG_MSG
    long seconds, useconds;
	#endif
	
	
    /*---------------------Buffer malloc -----------------*/	  
    unsigned char *rcbuffer = (unsigned char *)malloc(MTU); //1514
    unsigned char *buffer = (unsigned char *)malloc(5000000); //Its Big! 30000
    unsigned char * outBuffer = (char *)malloc(5000000);
    
    int s;
    OPENMAX_JPEG_DECODER *pDecoder1;
	OPENMAX_JPEG_DECODER *pDecoder2;
#ifdef DEBUG_MSG
    printf("Start\n");
#endif
    bcm_host_init();
    
    s = setupOpenMaxJpegDecoder(&pDecoder1);
	s = setupOpenMaxJpegDecoder(&pDecoder2);
#ifdef DEBUG_MSG
    printf("Decoder initialized\n");
#endif
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
	
    inet_adress = inet_addr(FRONTCAM);
    camMac = &camMacfront;
	
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
			//printf("Recvfrom: Daten empfangen\n");
		}
		#endif
		/*--------------------Prüfung ob Paket von einer wichtigen IP kommt ----- */
        
        struct ethhdr *edh = (struct ethhdr *)rcbuffer;
		if(checkMacAddr(&edh->h_source, camMac))
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
			if((ntohs(iph->frag_off) & IP_MF) == 0)
			{
			
				#ifdef DEBUG_MSG
				printf("Erste 3 Bits des IP Offset sind 000 \n");
				printf("IP Offset Flags: %d\n",(ntohs(iph->frag_off) & IP_MF));
				printf("IF IP Packet Size: %d\n", data_size);
				pakete++;
				printf("Anzahl der Pakete pro Bild: %d\n", pakete);
				
				#endif
			
				memcpy(buffer +index, rcbuffer+IP_HEADER+ETHERNET_HEADER , data_size-IP_HEADER-ETHERNET_HEADER);
				index += (data_size-IP_HEADER-ETHERNET_HEADER);
				
				#ifdef DEBUG_MSG
				//Empfangene Datei speichern
				//logfile = fopen("temp_img.jpg","w+");
				//fwrite(buffer, 1, index,logfile);
				//fclose(logfile);
				#endif
				
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
				#endif
				
				//erstes Paket verwerfen, koennte unvollständig sein
				if(firstPacket==0){
					
					int s;
					
					#ifdef DEBUG_MSG
					printf("Starte Decode\n");
					#endif
					
					if(inet_adress == inet_addr(FRONTCAM))
					{
						s = decodeImage(pDecoder1, buffer, (size_t)index, outBuffer );
					}
					else{
						s = decodeImage(pDecoder2, buffer, (size_t)index, outBuffer );
					}
					
					#ifdef DEBUG_MSG
					printf("Groeße nach Decode: %d\n",s);
					#endif
				
					if( s > 0 )
					{
						printf("Bild NICHT uebersprungen\n");
						/*-----------------------------------------------*/
						IplImage* image;
						image = cvCreateImageHeader(cvSize(320, 240), IPL_DEPTH_8U, 4); // Bild mit 320*240
						image->imageData = outBuffer;
					
						#ifdef DEBUG_MSG
						pictures++;
						sprintf(text, "%d", (int)pictures);
						cvPutText(image, text, cvPoint(280,15), &font, color);
						#endif

						cvShowImage("Simulator", image);
						#ifdef DEBUG_MSG
						printf("cvShowImage\n");
						#endif
						if ((cvWaitKey(5) & 255) == 27)	break;
						#ifdef DEBUG_MSG
						printf("cvWaitKey\n");
						#endif
						//(mouseClick == 0) ? (inet_adress = inet_addr(FRONTCAM)) : (inet_adress = inet_addr(BACKCAM));
						//(mouseClick == 0) ? (inet_adress = inet_addr(FRONTCAM)) : (inet_adress = inet_addr(BACKCAM)
                        (mouseClick == 0) ? (camMac = &camMacfront) : (camMac = camMacback);
						#ifdef DEBUG_MSG
						printf("mouseClick\n");
						#endif
					}
					
				
				}else{
					#ifdef DEBUG_MSG
					printf("Erstes Bild verworfen\n");
					#endif
					firstPacket=0;
				}
				
				/*-------- PI JPEG Hardwaredecode---------------*/
				
				

				//----------------------------------
				#ifdef DEBUG_MSG
				pakete=0;
				#endif
				index=0;
				erster_frame = 1;
				//----------------------------------
            }
			//Es folgen weitere Pakete
            else
            {
				#ifdef DEBUG_MSG
                pakete++;
				#endif
                if(erster_frame)
                {
#ifdef DEBUG_MSG
                    gettimeofday(&start, 0); //CPU-Zeit zu Beginn des Programmes
#endif
                    erster_frame = 0;
                }
#ifdef DEBUG_MSG
                printf("Erste 3 Bits des IP Offset sind NICHT 000 \n");
                printf("IP Offset Flags: %d\n",(ntohs(iph->frag_off) & IP_MF));
				
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
        else if(checkMacAddr(&edh->h_source, dataMac))
        {
            //doSomething
        }
    }
	
	close(sock_raw);
    printf("Finished");
}

int checkMacAddr(unsigned char * src, unsigned char * value){
    
    int i = 0;
    int resu = 1;
    for (i = 0; i < 6; i++) {
		#ifdef DEBUG_MSG
			//printf("Source MAC: %x - Value MAC: %x\n",src[i],value[i]);
		#endif
        if (src[i] != value[i]) {
            resu = 0;
        }
    }
    return resu;
}

int main(int argc, char *argv[]) 
{
    cvNamedWindow("Simulator", CV_WINDOW_AUTOSIZE);
    cvSetMouseCallback("Simulator", CallBackFunc, NULL);
	#ifdef DEBUG_MSG
	color = CV_RGB(255,0,0);
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 0.8,0.2,1,8);
	#endif
    IplImage *startImage;
    startImage  = cvLoadImage("start.jpg", CV_LOAD_IMAGE_COLOR);
    cvShowImage("Simulator", startImage);
    cvWaitKey(50);
    recv_func();
    cvReleaseImage(&startImage);
    
    return 0;     //nach terminierung aller Threads terminiert main
}
