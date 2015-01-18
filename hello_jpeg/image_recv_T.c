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
//Dest Mac
unsigned char dataMac[6] = {0x03, 0x04, 0x05, 0x06, 0x02, 0x9C};
unsigned char ethernet_can = {0x04;0x64;0x01;0x01;0x00;0x00;0x00;0x01;0x5e;0xc3;0x45;0x49;0x03;0x00;0x00;0x07;0xf1;0x00;0x08;0x00;0x00;0x14;0x40;0xff;0x7d;0xfd;0x01;0x00;0x00;0x00;0x01;0x5e;0xc3;0x4b;0xcc;0x03;0x00;0x00;0x07;0xf4;0x00;0x08;0x00;0x00;0x00;0x00;0x00;0x00;0x00;0x00;0x00;0x00;0x00;0x01;0x5e;0xc3;0x53;0xaa;0x03;0x00;0x00;0x07;0xf2;0x00;0x06;0x00;0xa6;0x07;0x2b;0xc6;0x20;0x00;0x00;0x00;0x01;0x5e;0xc3;0x5b;0x56;0x03;0x00;0x00;0x07;0xf3;0x00;0x08;0x00;0x00;0x00;0x00;0x00;0x00;0x00;0x00};

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
            //Paket wird als Ethernet geschickt (IP Header wird nicht benötigt)
            struct RTE_frame_payload *frame = (struct RTE_frame_payload *) (rcbuffer+ETHERNET_HEADER);
			
			printf("unit count (Anzahl TPUs): %s\n", frame->unit_count);
			printf("pack id(Sequenznummer): %s\n", frame->pack_id);
			printf("pack count(Anzahl der RTE Frames): %s\n", frame->pack_count);
			printf("pack number(Offset falls mehr als ein RTE): %s\n", frame->pack_number);
			
			
			int i = frame->unit_count;
			int offset = 0;
			//Anzahl der Durchläufe = Anzahl der TPUs
			for(i,i>0;i--){
				
				struct tpu *can_message = (struct tpu *) (rcbuffer+ETHERNET_HEADER+RTE_FRAME+offset);
			
				printf("encapsulation ID: %s\n", can_message->encapsulation_id);
				printf("time stamp: %s\n", can_message->time_stamp);
				printf("source bus ID: %s\n", can_message->source_bus_id);
				printf("CAN message ID: %s\n", can_message->can_message_id);
				printf("payload byte size: %s\n", can_message->payload_length);
				
				//Berechnen des Offset für den nächsten durchlauf (größe TPU Nachricht)
				offset = offset + RTE_TPU + can_message->payload_length;
				
				//rausschneiden der payload...
				
				char *array = malloc( sizeof(char) * ( can_message->payload_length) );
				array = (rcbuffer+ETHERNET_HEADER+RTE_FRAME+RTE_TPU);
				
				//funktioniert noch nicht (maskieren!)
				//00 00 14 40 ff 7d fd 01 hex
				//00000000 00000000 00010100 01000000 11111111 01111101 11111101 00000001
				//annahme: array sortierung ist: 0, 1, 2 ,... von links nach rechts
				if(can_message->can_message_id==MSG1){
					//VSIGNAL 16 BIT
					unint_64 vsignal = (int) (array & 0xFFFF000000000000);
					//Gierrate 14 BIT
					unint_64 gierrate = (int) (array & 0x0000FFFC00000000); << 16
					//VZ_Gierrate 1 BIT
					unint_64 vz_gierrate = (int) (array & 0x0000000200000000);
					//Stillstandsflag 1 BIT
					unint_64 car_standing = (int) (array & 0x0000000100000000);
					//Fahrtrichtung_HR 2 BIT
					unint_64 direction_hr = (int) (array & 0x00000000C0000000);
					//Fahrtrichtung_HL 2 BIT
					unint_64 direction_hl = (int) (array & 0x0000000030000000);
					//Fahrtrichtung_VR 2 BIT
					unint_64 direction_vr = (int) (array & 0x000000000C000000);
					//Fahrtrichtung_VL 2 BIT
					unint_64 direction_vl = (int) (array & 0x0000000003000000);
					//Laengsbeschleunigung 10 BIT
					unint_64 xlenght = (int) (array & 0x0000000000FFC000);
					//Querbeschleunigung 8 BIT
					unint_64 ylength = (int) (array & 0x0000000000003FC0);
				}
				else if(can_message->can_message_id==MSG2){
					//Fahrstufe 4 BIT
					unint_64 fahrstufe = (int) (array & 0xF00000000000);
					//Zaehnezahl 8 BIT
					unint_64 zaehnezahl = (int) (array & 0x0FF000000000);
					//Lenkradwinkel 13 BIT
					unint_64 lenkradwinkel = (int) (array & 0x000FFF800000);
					//VZ_Lenkradwinkel 1 BIT
					unint_64 vz_lenkradwinkel = (int) (array & 0x000000400000);
					//Reifenumfang 12 BIT
					unint_64 reifenumfang = (int) (array & 0x0000003FFC00);
					
				}
				else if(can_message->can_message_id==MSG3){
					//Radgeschw_VR 16 BIT
					unint_64 vrad_vr = (int) (array & 0xFFFF000000000000);
					//Radgeschw_VL 16 BIT
					unint_64 vrad_vl = (int) (array & 0x0000FFFF00000000);
					//Radgeschw_HR 16 BIT
					unint_64 vrad_hr = (int) (array & 0x00000000FFFF0000);
					//Radgeschw_HL 16 BIT
					unint_64 vrad_hl = (int) (array & 0x000000000000FFFF);

				}
				else if(can_message->can_message_id==MSG4){
					//Wegimpuls_HR 10 BIT
					unint_64 wegimpuls_hr = (int) (array & 0xFFC0000000000000);
					//Wegimpuls_HL 10 BIT
					unint_64 wegimpuls_hl = (int) (array & 0x003FF00000000000);
					//Wegimpuls_VR 10 BIT
					unint_64 wegimpuls_vr = (int) (array & 0x00000FFC00000000);
					//Wegimpuls_VL 10 BIT
					unint_64 wegimpuls_vl = (int) (array & 0x00000003FF000000);
				}
				
				free(array);
			}
}
			
			
			
			
			
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
