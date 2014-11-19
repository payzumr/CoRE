/*
-----Patrick Kuncke
-----21.04.2013

*/
#ifndef _HEADER_H
#define	_HEADER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/time.h>

//Buffergröße bei 20 fps kann somit 5 sec gebuffert werden
#define BUFFER_SIZE 60
#define SERVPORT 5000
#define SERVER_PORT	9999
#define SERVER "localhost"
#define IMAGESIZE  230400 //320 * 230 * 3;
#define MAX_CLIENTS 10
// #define DEBUG_MESSAGES

#define THREADERR(returnVal, errortext) if((returnVal) != 0){errno = returnVal; perror(errortext);}    //Makro fuer Fehlermeldung mit errno oder 0 r�ckgabe
#define SEMERR(returnVal, errortext) if((returnVal) == -1){perror(errortext);}      //Makro fuer Fehlermeldung mit -1 oder 0

void buffer_init();
int put(IplImage* image);
IplImage* get(int client_pointer, int index);
int get_server_status();

#endif

