#include <stdio.h> //For standard things
#include <stdlib.h>    //malloc
#include <string.h>    //memset
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <errno.h>

#include <gtk/gtk.h>

#define UDP_PROTO 17
#define IP_HEADER_MAX 65536
#define FRONTCAM "10.10.1.11"
#define BACKCAM "10.10.1.12"



extern GtkWidget *image;
int inet_adress;

void recv_func(gpointer data);

//extern static gpointer thread_func( gpointer data );
