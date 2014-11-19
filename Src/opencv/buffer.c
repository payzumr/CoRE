/*
 * buffer.c
 *
 *  Created on: 22.10.2013
 *      Author: Patrick Kuncke 2094460
 *				Tim-Ole Schaffeld
 */
#include "header.h"

/* Variablen */
//Größe des Buffer
IplImage *imageArr[BUFFER_SIZE];
IplImage *return_img;
int server_pointer = 0; //pointer zum schreiben in den puffer
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//	Array initializieren und speicher allokieren für Images
void buffer_init(){
	//Rückgabe eines neu erstellten Bildes
	return_img = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
}

int put(IplImage* image) { //legt neues Image in den puffer
	pthread_mutex_lock(&mutex);
	//der Speicher des aktuellen Bildes wird frei gegeben
    cvReleaseImage(&imageArr[server_pointer]);
	//neues Bild an der Stelle vom Serverpointer erstellen
    imageArr[server_pointer] = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	cvCopy(image, imageArr[server_pointer], NULL);
	//pointer zahl erhöhen und ueberlauf verhindern
	server_pointer = (server_pointer + 1) % BUFFER_SIZE; 
	//Mutex freigeben
    pthread_mutex_unlock(&mutex);
	return 1;
}

//Bild aus dem Buffer zurückliefern an der Stelle des Clientpointers
IplImage* get(int client_pointer, int index) {
	//Mutex locken (kritischer Bereich)
	pthread_mutex_lock(&mutex);
	//Kopieren des Bildes aus dem Puffer in eine temporäre Variable
	return_img = imageArr[client_pointer];
	//verlassen des kritischen Bereichs (Mutex unlock)
    pthread_mutex_unlock(&mutex);
    //Bild zurück geben
	return return_img;
}

//gibt die akuelle Schreibposition des Servers zurueck (aktuell)
int get_server_status() { 
	return server_pointer;
}