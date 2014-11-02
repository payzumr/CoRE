#ifndef NETWORK_HELPER_H
#define NETWORK_HELPER_H

#include <QObject>

#include <sys/socket.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>

typedef struct __attribute__((__packed__)) ip_header       // 20 Bytes
{
    quint8  iph_ihlVer;           // Version + Header-length
    quint8  iph_tos;              // Type of service
    quint16 iph_len;              // Total length
    quint16 iph_ident;            // Identification
    //quint8  iph_flags;            // Flags (3 bits)
    quint16 iph_offset;           // Fragment offset (13 bits)
    quint8  iph_ttl;              // Time to live
    quint8  iph_protocol;         // Protocol
    quint16 iph_chksum;           // Header checksum
    quint32 iph_sourceip;         // Source address
    quint32 iph_destip;           // Destination address
} ip_header;


/*class Ip_Helper : public QObject
{
    Q_OBJECT

public:
    Ip_Helper();


private:

};*/

typedef struct mac_address  // 6 Bytes
{
    quint8 Byte1;
    quint8 Byte2;
    quint8 Byte3;
    quint8 Byte4;
    quint8 Byte5;
    quint8 Byte6;
} mac_address;

typedef struct ip_address   // 4 Bytes
{
    quint8 Byte1;
    quint8 Byte2;
    quint8 Byte3;
    quint8 Byte4;
} ip_address;

void ethernet_prepareHeader(struct ethhdr* eh, struct sockaddr_ll* socket_address);
void ip_prepareHeader(ip_header* ih);
void ip_setMoreFragments(ip_header* ih, bool moreFragments);

#endif // NETWORK_HELPER_H
