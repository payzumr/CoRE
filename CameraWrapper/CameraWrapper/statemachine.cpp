#include "statemachine.h"

#include "network_helper.h"

#include <QCoreApplication>
#include <QTime>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <unistd.h>


#include <errno.h>

//-----include von Paddy und Phil
#include <stdio.h>
// --------------------------------------------------public----------------------------------------
Statemachine::Statemachine(Arguments *arguments) :
    QThread()
{
    struct ifreq pInterfaceId;

    this->arguments = new Arguments(arguments);

    // create RAW ethernet socket
    if ((sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    //if ((sd = socket(PF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        // exception has to be thrown
        qCritical("unable to open Socket");
        exit(EXIT_FAILURE);
    }
    // Bind socket to device
    if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, this->arguments->interfaceName.toUtf8().constData(), IFNAMSIZ) == -1)
    {
        // exception has to be thrown
        qCritical("unable to bind socket to interface");
        close(sd);
        exit(EXIT_FAILURE);
    }
    // get interface index
    memset(&pInterfaceId, 0x00, sizeof(pInterfaceId));
    strncpy(pInterfaceId.ifr_name, this->arguments->interfaceName.toUtf8().constData(), IFNAMSIZ);
    if (ioctl(sd, SIOCGIFINDEX, &pInterfaceId) < 0)
    {
        // exception has to be thrown
        qCritical("unable to get interface by index");
        close(sd);
        exit(EXIT_FAILURE);
    }
    ucInterfaceIndex = pInterfaceId.ifr_ifindex;
}

/*
Statemachine::Statemachine(Arguments *arguments) :
    QThread()
{
    // pointer to kernel signals
    int one = 1;
    const int *val = &one;

    this->arguments = new Arguments(arguments);

    // Create a raw socket with IP protocol
    //sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sd < 0)
    {
        // exception has to be thrown
        qCritical("unable to open Socket");
        exit(EXIT_FAILURE);
    }
    // inform the kernel do not fill up the headers' structure
    /*if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
    {
        // exception has to be thrown
        qCritical("unable to signal Kernel");
        exit(EXIT_FAILURE);
    }*/
    /*int broadcast=1;
    if (setsockopt(sd,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast))==-1)
    {
        // exception has to be thrown
        qCritical("unable to signal Kernel");
        exit(EXIT_FAILURE);
    }*/

//}


Statemachine::~Statemachine()
{
   delete this->arguments;
}


void Statemachine::run()
{
    //#define ETH_FRAME_LEN   1518

    #define ETH_HEADER_LEN    14
    #define IP_HEADER_LEN     20
    #define ETH_CRC_LEN        4

    #define DATAGRAM_SIZE  30000
    #define MTU_SIZE     (8 * 185)


    struct sockaddr_ll socket_address;

    // framebuffer
    void* videoData = (void*)malloc(DATAGRAM_SIZE);
    quint8* datagram = (quint8*)videoData;
    void* etherFrame = (void*)malloc(ETH_FRAME_LEN - ETH_CRC_LEN);
    quint8* etherhead = (quint8*)etherFrame;
    quint8* iphead = (quint8*)etherFrame + ETH_HEADER_LEN;
    quint8* data = (quint8*)etherFrame + ETH_HEADER_LEN + IP_HEADER_LEN;
	char string[18] = "./lache/img00.jpg";
	FILE *fi;
    // header structure
    //struct ethhdr *eh = (struct ethhdr*)etherhead;
    ip_header *ptIh = (ip_header*)iphead;
    // header buffers
    struct ethhdr eh;
    ip_header ih;
	int imageLength;

    int send_result = 0;
    quint16 usBytesToSend;
    quint16 usEtherSize;
    quint16 usFragmentOffset = 0;
    QString result;

    // prepare ethernet header structures
    ethernet_prepareHeader(&eh, &socket_address);
    socket_address.sll_ifindex = ucInterfaceIndex;
	
	int iCount;
	iCount = 0;

    while (true)
    {
        // prepare ip header structures
        ip_prepareHeader(&ih);
        // get video data
		
		sprintf(string, "./lache/img%02d.jpg", iCount +1);
		
		fi = fopen (string, "rb+");
  		if (fi == NULL) {
    		printf ("Can't open file 'data'.\n");
    		exit (EXIT_FAILURE);
  		}
  		
				iCount = (iCount + 1) % 91;
 		//get its size
		fseek(fi, 0, SEEK_END);
		imageLength = ftell(fi);

 		//read image data into buffer
 		fread ((quint8*)videoData,1,imageLength,fi);
		
		
  		fclose (fi);
		
		
		
		
        // usBytesToSend = procGetFrame(videoData);
        usBytesToSend = (quint16) imageLength;

        do
        {
            // write header information to frame
            memcpy((void*)etherhead, (void*)&eh, ETH_HEADER_LEN);
            memcpy((void*)iphead, (void*)&ih, IP_HEADER_LEN);
            // calculate fragmentation
            if (usBytesToSend < MTU_SIZE)
            {
                usEtherSize = usBytesToSend;
                usBytesToSend = 0;
                usFragmentOffset = 0;
            }
            else
            {
                usEtherSize = MTU_SIZE;
                usBytesToSend = usBytesToSend - MTU_SIZE;
                // update ip header
                ip_setMoreFragments(ptIh, true);
                usFragmentOffset = usFragmentOffset + (MTU_SIZE / 8);
            }
            ptIh->iph_offset |= usFragmentOffset;
			
            // write data
            memcpy((void*)data, (void*)(datagram + (DATAGRAM_SIZE - (usBytesToSend + usEtherSize))), usEtherSize);
            // send RAW Ethernet frame
            send_result = sendto(sd, etherFrame, usEtherSize + ETH_HEADER_LEN + IP_HEADER_LEN, 0, (struct sockaddr*)&socket_address, (socklen_t)sizeof(socket_address));
            if (send_result == -1)
                qWarning("frameloss due to send failure");
        }
        while (usBytesToSend != 0);
        delay();
    }
}


/*void Statemachine::run()
{
    #define PCKT_LEN 100//8192

    //const char* ipSrc = "10.10.1.100";
    const char* ipSrc = "192.168.0.100";
    //const char* ipDest = "10.10.1.12";
    const char* ipDest = "192.168.0.12";
    int send_result = 0;

    struct sockaddr_in sin, din;

    // definition of datagram
    char buffer[PCKT_LEN];

    // header structure
    ip_header *ip = (struct ip_header *) buffer;

    // The address family
    sin.sin_family = AF_INET;
    din.sin_family = AF_INET;
    // IP addresses
    sin.sin_addr.s_addr = inet_addr(ipSrc);
    din.sin_addr.s_addr = inet_addr(ipDest);

    // create the IP header
    ip->iph_ihl = 5;
    ip->iph_ver = 4;
    ip->iph_tos = 16; // Low delay
    ip->iph_len = sizeof(struct ip_header);
    ip->iph_ident = htons(54321);
    ip->iph_ttl = 64; // hops
    ip->iph_protocol = 0;//17; // UDP
    ip->iph_sourceip = inet_addr(ipSrc);
    ip->iph_destip = inet_addr(ipDest);
    // IP checksum calculation
    ip->iph_chksum = csum((unsigned short *) buffer, sizeof(struct ip_header));

    while (createframe(quint8* ptDatagram, quint8* ptFrame, quint32 ulStart, quint16 usMtu) )


    while (true)
    {
        // send RAW IP frame
        //send_result = sendto(sd, buffer, ip->iph_len, 0, (struct sockaddr*)&sin, (socklen_t)sizeof(sin));

        send_result = sendto(sd, buffer, PCKT_LEN, 0, (struct sockaddr*)&sin, (socklen_t)sizeof(sin));

        if (send_result == -1)
            qWarning("frameloss due to send failure");
        else
            qWarning("sending frame");
        delay();
    }
}*/


// --------------------------------------------------private----------------------------------------
void Statemachine::delay()
{
    QTime dieTime = QTime::currentTime().addMSecs(arguments->ulIntervalMs);
    while (QTime::currentTime() < dieTime)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}



/*#include "statemachine.h"

Statemachine::Statemachine(QObject *parent) :
    QObject(parent)
{
}



 void MyObject::startWorkInAThread()
 {
     WorkerThread *workerThread = new WorkerThread(this);
     connect(workerThread, SIGNAL(resultReady(QString)), this, SLOT(handleResults(QString)));
     connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
     workerThread->start();
 }



void Statemachine::doWork()
{
    for (int i = 0; i < 60; i ++) {

        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if (abort) {
            break;
        }

        // This will stupidly wait 1 sec doing nothing...
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();

        emit valueChanged(QString::number(i));
 }

mutex.lock();
_working = false;
mutex.unlock();

 emit finished();
}
*/
