#include "network_helper.h"

unsigned short csum(unsigned short *buf, int nwords);

void ethernet_prepareHeader(struct ethhdr* eh, struct sockaddr_ll* socket_address)
{
    quint8 src_mac[6]  = {0xd4, 0xbe, 0xd9, 0x69, 0xca, 0xb5};
    quint8 dest_mac[6]  = {0x03, 0x04, 0x05, 0x06, 0x04, 0x01};

    // RAW communication
    memset((void*)socket_address, 0, sizeof(sockaddr_ll));
    socket_address->sll_family = PF_PACKET;
    socket_address->sll_protocol = htons(ETH_P_ALL);
    socket_address->sll_hatype   = ARPHRD_ETHER;
    socket_address->sll_pkttype  = PACKET_OTHERHOST;

    // setting destination address
    socket_address->sll_halen = ETH_ALEN;
    memcpy((void*)socket_address->sll_addr, (void*)dest_mac, ETH_ALEN);

    // set the frame headerinformation
    memcpy((void*)eh->h_dest, (void*)dest_mac, ETH_ALEN);
    memcpy((void*)eh->h_source, (void*)src_mac, ETH_ALEN);
    eh->h_proto = htons(0x0800);
}

void ip_prepareHeader(ip_header* ih)
{
    const char* ipSrc = "10.10.1.11";
    const char* ipDest = "10.10.1.100";
    static quint16 usIdentifier = 54321;

    // create the IP header
    ih->iph_ihlVer = (4 << 4) | 5;
    ih->iph_tos = 16; // Low delay
    ih->iph_len = htons(sizeof(struct ip_header));
    ih->iph_ident = htons(usIdentifier++);
    ih->iph_offset = 0;
    ih->iph_ttl = 64; // hops
    ih->iph_protocol = 0;//17 UDP
    ih->iph_sourceip = inet_addr(ipSrc);
    ih->iph_destip = inet_addr(ipDest);
    // IP checksum calculation
    ih->iph_chksum = 0;
    ih->iph_chksum = csum((unsigned short*)ih, sizeof(struct ip_header));
}

void ip_setMoreFragments(ip_header* ih, bool moreFragments)
{
    #define FLAG_MF 0x020

    if (moreFragments == true)
        ih->iph_offset |= FLAG_MF;
    else
		
        ih->iph_offset &= ~FLAG_MF;
}


/*
Ip_Helper::Ip_Helper(QObject *parent) :
    QObject(parent)
{



}
*/

// Function for checksum calculation. From the RFC,
// The checksum algorithm is:
// "The checksum field is the 16 bit one's complement of the one's
// complement sum of all 16 bit words in the header.  For purposes of
// computing the checksum, the value of the checksum field is zero."
unsigned short csum(unsigned short* buf, int nwords)
{
    quint32 ulSum;

    for (ulSum = 0; nwords > 0; nwords--)
        ulSum += *buf++;
    ulSum = (ulSum >> 16) + (ulSum &0xffff);
    ulSum += (ulSum >> 16);
    return (quint16)(~ulSum);
}

