#include <stdio.h> //For standard things
#include <stdlib.h>    //malloc
#include <string.h>    //memset
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <sys/time.h>
#include <time.h>
#include "jpeg.h"

//OpenCV
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define IMG_MAX 30000
#define MTU 1514
#define IP_HEADER sizeof(struct iphdr)
#define ETHERNET_HEADER sizeof(struct ethhdr)

#define FRONTCAM "10.10.1.11"
#define BACKCAM "10.10.1.12"

int inet_adress;



