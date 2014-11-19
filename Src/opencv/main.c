/*
 -----Patrick Kuncke
 -----21.04.2013
 */
#include "header.h"

//Verwaltungsstruktur für die Clients
struct semthreads{
	//Socket für die TCP Verbindung des Clients
    int sock;
    //Anzahl der im Puffer vorhandenen Bilder, die noch nicht angezeigt wurden
    sem_t entries;
    //Pointer an dem der Client gerade ist
    int array_pointer;
};

/* Variablen */
pthread_t dummy;
//sem fur maximale anzahl der clients
sem_t clients;
//Variable für den Hostname des Servers mit dem wir uns verbinden wollen
char hostname[255];
int client_count = 0;
//Array mit Verwaltungsstrukturen für die Clients
struct semthreads *client_p_c[MAX_CLIENTS];



void* anzeigeThread(void* arg) {
	
	int sockD; // socket descriptor
	int recvBytes; //recv
	struct sockaddr_in serveraddr;
	struct hostent *hostp;
	
	buffer_init();
	/*
	*	socket()
	* AF_INET für ipv4
	* SOCK_STREAM für TCP Protokoll
	* TCP ist durch den vorigen Parameter eindeutig deswegen die 0
	*/
	/* get a socket descriptor */
	if ((sockD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Client-Socket() Error");
		exit(-1);
	} else
		printf("Client-Socket() OK\n");
	
	#ifdef DEBUG_MESSAGES
	printf("socked erstellt mit nummer : %d \n", sockD);
	#endif
	
	memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVPORT);
		
	//Umwandlung des übergebenen Hostnames in die Binärschreibweise	(vergleich auf -1 (-1 = umwandlung fehlgeschlagen))
	//Umwandlung funktioniert nur wenn bereits eine IP Addresse übergeben wurde
	if ((serveraddr.sin_addr.s_addr = inet_addr(hostname)) == (unsigned long) INADDR_NONE ) {

		/* When passing the host name of the server as a */
		/* parameter to this program, use the gethostbyname() */
		/* function to retrieve the address of the host server. */
		/***************************************************/
		/* get host address */
		hostp = gethostbyname(hostname);
		//Prüfen ob eine Adresse zurück geliefert wurde
		if (hostp == (struct hostent *) NULL ) {
			printf("HOST NOT FOUND --> ");
			/* h_errno is usually defined */
			/* in netdb.h */
			printf("h_errno = %d\n", h_errno);
			printf("---This is a client program---\n");
			printf("Command usage: %s <server name or IP>\n", hostname);
			close(sockD);
			exit(-1);
		}
		memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
	}
	
	#ifdef DEBUG_MESSAGES
	printf("bereit zum connect \n");
	#endif
	
	/* After the socket descriptor is received, the */
	/* connect() function is used to establish a */
	/* connection to the server. */
	/***********************************************/

	/* connect() to server. */
	if ((connect(sockD, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
		    < 0) {
		perror("Client-connect() error");
		close(sockD);
		exit(-1);
		    } else
			    printf("Connection established...\n");
	
	#ifdef DEBUG_MESSAGES
	printf("connect ausgeführt \n");
	#endif
	
	//Fenster öffnen zur Darstellung der Bilder
	cvNamedWindow("Simulator", CV_WINDOW_AUTOSIZE);
	printf("Fenster erstellt.\n");
	
	
	while(1){
		IplImage* image;
		image = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3); // Bild mit 320*240
		recvBytes = 0;
		//recv() empfängt bis die gesamte Daten da sind
		while (recvBytes < IMAGESIZE) { //warten bis ein volles bild empfangen wurde
			int bytes = 0;
			bytes = recv(sockD, &image->imageData[recvBytes], image->imageSize - recvBytes, 0);
			if (bytes == -1) {
				perror("recv call failed");
				break;
			}
			if (bytes == 0) {
				perror("connection was closed ");
				break;
			} 	//connection was closed
			//printf("recvBytes = %d\n", recvBytes);
			recvBytes += bytes;
		}
		//printf("gesamt = %d\n", recvBytes);
		//Bild im Puffer speichern
		put(image);
		//Bild mit Hilfe der OpenCV Bibliothek im Fester anzeigen
		cvShowImage("Simulator", image);

		if ((cvWaitKey(5) & 255) == 27)
			break;
		
		int j;
		//Semaphore von jedem Client erhöhen der aktiv ist (!=NULL im Array)
        for(j = 0; j < MAX_CLIENTS; j++){
            if(client_p_c[j] != NULL){
				#ifdef DEBUG_MESSAGES
					printf("sem_post gemacht von %d\n", j);
				#endif
                sem_post( &client_p_c[j]->entries );
            }
        }
		//Speicher freigeben
		cvReleaseImage(&image);
	}
	
		//close
	recvBytes = close(sockD);
	if (recvBytes == -1) {
		perror("close call failed");
		exit(1);
	}
	return 0;
}

int main(int argc, char *argv[]) {
	// Join wartet das sich alle Threads beendet haben
	pthread_join(dummy, NULL);
	sem_destroy(&clients);
	printf("erfolgreich beendet!!!\n");
	return (EXIT_SUCCESS);     //nach terminierung aller Threads terminiert main
}


