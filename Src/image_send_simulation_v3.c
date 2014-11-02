/*  Copyright (C) 2012-2013  P.D. Buchan (pdbuchan@yahoo.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Send an IPv4 UDP packet via raw socket at the link layer (ethernet frame)
// with a large payload requiring fragmentation.
// Need to have destination MAC address.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_UDP, INET_ADDRSTRLEN
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/udp.h>      // struct udphdr
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>

#include <errno.h>            // errno, perror()

// Define some constants.
#define ETH_HDRLEN 14         // Ethernet header length
#define IP4_HDRLEN 20         // IPv4 header length
#define UDP_HDRLEN  8         // UDP header length, excludes data
#define MAX_FRAGS 3120        // Maximum number of packet fragments (int) (65535 - UDP_HDRLEN) / (IP4_HDRLEN + 1 data byte))

// Function prototypes
uint16_t checksum (uint16_t *, int);
uint16_t udp4_checksum (struct ip, struct udphdr, uint8_t *, int);
char *allocate_strmem (int);
uint8_t *allocate_ustrmem (int);
int *allocate_intmem (int);

int
main (int argc, char **argv)
{
	int i, n, status, frame_length, sd, bytes, iCount, iGesamt;
  	int *ip_flags, mtu, c, nframes, offset[MAX_FRAGS], len[MAX_FRAGS];
  	char *interface, *target, *src_ip, *dst_ip;
  	struct ip iphdr;
  	struct udphdr udphdr;
  	int payloadlen, bufferlen;
  	uint8_t *payload, *buffer, *src_mac, *dst_mac, *ether_frame;
  	struct addrinfo hints, *res;
  	struct sockaddr_in *ipv4;
  	struct sockaddr_ll device;
  	struct ifreq ifr;
  	void *tmp;
  	FILE *fi;
	char string[17] = "./lache/img00.jpg";
	
	iCount = 0;
	iGesamt = 0;

  	// Allocate memory for various arrays.
  	src_mac = allocate_ustrmem (6);
  	dst_mac = allocate_ustrmem (6);
  	payload = allocate_ustrmem (IP_MAXPACKET);
  	ether_frame = allocate_ustrmem (IP_MAXPACKET);
  	interface = allocate_strmem (40);
  	target = allocate_strmem (40);
  	src_ip = allocate_strmem (INET_ADDRSTRLEN);
  	dst_ip = allocate_strmem (INET_ADDRSTRLEN);
  	ip_flags = allocate_intmem (4);

	while(1){
  
  		// Interface to send packet through.
  		strcpy (interface, "eth0");

  		// Submit request for a socket descriptor to look up interface.
  		if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
    		perror ("socket() failed to get socket descriptor for using ioctl() ");
    		exit (EXIT_FAILURE);
  		}

  		// Use ioctl() to get interface maximum transmission unit (MTU).
  		memset (&ifr, 0, sizeof (ifr));
  		strcpy (ifr.ifr_name, interface);
  		if (ioctl (sd, SIOCGIFMTU, &ifr) < 0) {
    		perror ("ioctl() failed to get MTU ");
    		return (EXIT_FAILURE);
  		}
  		mtu = ifr.ifr_mtu;
#ifdef DEBUG_MSG
  		printf ("Current MTU of interface %s is: %i\n", interface, mtu);
#endif
  		// Use ioctl() to look up interface name and get its MAC address.
  		memset (&ifr, 0, sizeof (ifr));
  		snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
  		if (ioctl (sd, SIOCGIFHWADDR, &ifr) < 0) {
    		perror ("ioctl() failed to get source MAC address ");
    		return (EXIT_FAILURE);
  		}
  		close (sd);

  		// Copy source MAC address.
  		memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));

		// Report source MAC address to stdout.
#ifdef DEBUG_MSG
  		printf ("MAC address for interface %s is ", interface);
  		for (i=0; i<5; i++) {
    		printf ("%02x:", src_mac[i]);
  		}
  		printf ("%02x\n", src_mac[5]);
#endif

  		// Find interface index from interface name and store index in
  		// struct sockaddr_ll device, which will be used as an argument of sendto().
  		memset (&device, 0, sizeof (device));
  		if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
    		perror ("if_nametoindex() failed to obtain interface index ");
    		exit (EXIT_FAILURE);
  		}
#ifdef DEBUG_MSG
  		printf ("Index for interface %s is %i\n", interface, device.sll_ifindex);
#endif

  		// Set destination MAC address: you need to fill these out
  		dst_mac[0] = 0xff;
  		dst_mac[1] = 0xff;
  		dst_mac[2] = 0xff;
  		dst_mac[3] = 0xff;
  		dst_mac[4] = 0xff;
  		dst_mac[5] = 0xff;

  		// Source IPv4 address: you need to fill this out
  		strcpy (src_ip, "10.10.1.11");

  		// Destination URL or IPv4 address
  		strcpy (target, "127.0.0.1");

  		// Fill out hints for getaddrinfo().
  		memset (&hints, 0, sizeof (struct addrinfo));
  		hints.ai_family = AF_INET;
  		hints.ai_socktype = SOCK_RAW;
  		hints.ai_flags = hints.ai_flags | AI_CANONNAME;

		// Resolve target using getaddrinfo().
		if ((status = getaddrinfo (target, NULL, &hints, &res)) != 0) {
    		fprintf (stderr, "getaddrinfo() failed: %s\n", gai_strerror (status));
    		exit (EXIT_FAILURE);
  		}
  		ipv4 = (struct sockaddr_in *) res->ai_addr;
  		tmp = &(ipv4->sin_addr);
  		if (inet_ntop (AF_INET, tmp, dst_ip, INET_ADDRSTRLEN) == NULL) {
    		status = errno;
    		fprintf (stderr, "inet_ntop() failed.\nError message: %s", strerror(status));
    		exit (EXIT_FAILURE);
  		}
  		freeaddrinfo (res);

  		// Fill out sockaddr_ll.
  		device.sll_family = AF_PACKET;
  		memcpy (device.sll_addr, src_mac, 6 * sizeof (uint8_t));
  		device.sll_halen = htons (6);

  		// Get UDP data.
		
		sprintf(string, "./lache/img%02d.jpg", iCount +1);
			
#ifdef FRONT
		strcpy (src_ip, "10.10.1.11");
#endif
#ifdef BACK
		strcpy (src_ip, "10.10.1.12");
#endif
		
  		fi = fopen (string, "rb+");
  		if (fi == NULL) {
    		printf ("Can't open file 'data'.\n");
    		exit (EXIT_FAILURE);
  		}
  		
				iCount = (iCount + 1) % 91;
 		//get its size
		fseek(fi, 0, SEEK_END);
		payloadlen = ftell(fi);
 		rewind (fi);

 		//malloc the buffer big enough for image data

 		payload = (char*) malloc (payloadlen);
 		//read image data into buffer
 		fread (payload,1,payloadlen,fi);
  
 		if(payloadlen > 65000)
		{
#ifdef DEBUG_MSG
	 		printf("Error File zu groß!!!");
#endif
 		}
#ifdef DEBUG_MSG		
 		printf("Bildgroeße: %d\n", payloadlen); 
#endif
		
		
  		fclose (fi);
#ifdef DEBUG_MSG
  		printf ("Upper layer protocol header length (bytes): %i\n", UDP_HDRLEN);
  		printf ("Payload length (bytes): %i\n", payloadlen);
#endif
  		// Length of fragmentable portion of packet.
  		bufferlen = UDP_HDRLEN + payloadlen;
#ifdef DEBUG_MSG
  		printf ("Total fragmentable data (bytes): %i\n", bufferlen);
#endif
  		// Allocate memory for a buffer for fragmentable portion.
  		buffer = allocate_ustrmem (bufferlen);

  		// Determine how many ethernet frames we'll need.
  		memset (len, 0, MAX_FRAGS * sizeof (int));
  		memset (offset, 0, MAX_FRAGS * sizeof (int));
  		i = 0;
  		c = 0;  // Variable c is index to buffer, which contains upper layer protocol header and data.
  		while (c < bufferlen) {

    		// Do we still need to fragment remainder of fragmentable portion?
    		if ((bufferlen - c) > (mtu - IP4_HDRLEN)) {  // Yes
      			len[i] = mtu - IP4_HDRLEN;  // len[i] is amount of fragmentable part we can include in this frame.

    		} 
			else 
			{  // No
      			len[i] = bufferlen - c;  // len[i] is amount of fragmentable part we can include in this frame.
    		}
    		c += len[i];

    		// If not last fragment, make sure we have an even number of 8-byte blocks.
    		// Reduce length as necessary.
    		if (c < (bufferlen - 1)) {
      			while ((len[i]%8) > 0) {
        			len[i]--;
        			c--;
      			}
    		}
#ifdef DEBUG_MSG
    		printf ("Frag: %i,  Data (bytes): %i,  Data Offset (8-byte blocks): %i\n", i, len[i], offset[i]);
#endif
    		i++;
    		offset[i] = (len[i-1] / 8) + offset[i-1];
  		}
  		nframes = i;
#ifdef DEBUG_MSG
  		printf ("Total number of frames to send: %i\n", nframes);
#endif
  		// IPv4 header

  		// IPv4 header length (4 bits): Number of 32-bit words in header = 5
  		iphdr.ip_hl = IP4_HDRLEN / sizeof (uint32_t);

  		// Internet Protocol version (4 bits): IPv4
  		iphdr.ip_v = 4;

  		// Type of service (8 bits)
  		iphdr.ip_tos = 0;

  		// Total length of datagram (16 bits)
  		// iphdr.ip_len is set for each fragment in loop below.

  		// ID sequence number (16 bits)
  		iphdr.ip_id = htons (31415);

  		// Flags, and Fragmentation offset (3, 13 bits)

  		// Zero (1 bit)
  		ip_flags[0] = 0;

  		// Do not fragment flag (1 bit)
  		ip_flags[1] = 0;

  		// More fragments following flag (1 bit)
  		ip_flags[2] = 0;

  		// Fragmentation offset (13 bits)
  		ip_flags[3] = 0;

  		iphdr.ip_off = htons ((ip_flags[0] << 15)
                      		+ (ip_flags[1] << 14)
                      		+ (ip_flags[2] << 13)
                     		+  ip_flags[3]);

  		// Time-to-Live (8 bits): default to maximum value
  		iphdr.ip_ttl = 255;

  		// Transport layer protocol (8 bits): 17 for UDP
  		iphdr.ip_p = IPPROTO_UDP;

  		// Source IPv4 address (32 bits)
  		if ((status = inet_pton (AF_INET, src_ip, &(iphdr.ip_src))) != 1) {
    		fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    		exit (EXIT_FAILURE);
  		}

  		// Destination IPv4 address (32 bits)
  		if ((status = inet_pton (AF_INET, dst_ip, &(iphdr.ip_dst))) != 1) {
    		fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    		exit (EXIT_FAILURE);
  		}

  		// IPv4 header checksum (16 bits): set to 0 when calculating checksum
  		iphdr.ip_sum = 0;
  		iphdr.ip_sum = checksum ((uint16_t *) &iphdr, IP4_HDRLEN);

  		// UDP header

  		// Source port number (16 bits): pick a number
  		udphdr.source = htons (4950);

  		// Destination port number (16 bits): pick a number
  		udphdr.dest = htons (4950);

  		// Length of UDP datagram (16 bits): UDP header + UDP data
  		udphdr.len = htons (UDP_HDRLEN + payloadlen);

  		// UDP checksum (16 bits)
  		udphdr.check = udp4_checksum (iphdr, udphdr, payload, payloadlen);

  		// Build fragmentable portion of packet in buffer array.
  		// UDP header
  		memcpy (buffer, &udphdr, UDP_HDRLEN * sizeof (uint8_t));
  		// UDP data
  		memcpy (buffer + UDP_HDRLEN, payload, payloadlen * sizeof (uint8_t));

  		// Submit request for a raw socket descriptor.
  		if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
   			perror ("socket() failed ");
    		exit (EXIT_FAILURE);
  		}

  		// Loop through fragments.
  		for (i=0; i<nframes; i++) {

    		// Set ethernet frame contents to zero initially.
    		memset (ether_frame, 0, IP_MAXPACKET * sizeof (uint8_t));

    		// Fill out ethernet frame header.

    		// Copy destination and source MAC addresses to ethernet frame.
    		memcpy (ether_frame, dst_mac, 6 * sizeof (uint8_t));
    		memcpy (ether_frame + 6, src_mac, 6 * sizeof (uint8_t));

    		// Next is ethernet type code (ETH_P_IP for IPv4).
    		// http://www.iana.org/assignments/ethernet-numbers
    		ether_frame[12] = ETH_P_IP / 256;
    		ether_frame[13] = ETH_P_IP % 256;

    		// Next is ethernet frame data (IPv4 header + fragment).

    		// Total length of datagram (16 bits): IP header + fragment
    		iphdr.ip_len = htons (IP4_HDRLEN + len[i]);

    		// More fragments following flag (1 bit)
    		if ((nframes > 1) && (i < (nframes - 1))) {
     			ip_flags[2] = 1u;
    		} else {
      			ip_flags[2] = 0u;
    		}

    		// Fragmentation offset (13 bits)
    		ip_flags[3] = offset[i];

    		// Flags, and Fragmentation offset (3, 13 bits)
    		iphdr.ip_off = htons ((ip_flags[0] << 15)
                        		+ (ip_flags[1] << 14)
                        		+ (ip_flags[2] << 13)
                        		+  ip_flags[3]);
			
			printf("IP Offsetfeld   : %d\n",ntohs(iphdr.ip_off));
			
    		// IPv4 header checksum (16 bits)
    		iphdr.ip_sum = 0;
    		iphdr.ip_sum = checksum ((uint16_t *) &iphdr, IP4_HDRLEN);

    		// Copy IPv4 header to ethernet frame.
    		memcpy (ether_frame + ETH_HDRLEN, &iphdr, IP4_HDRLEN * sizeof (uint8_t));

    		// Copy fragmentable portion of packet to ethernet frame.
    		memcpy (ether_frame + ETH_HDRLEN + IP4_HDRLEN, buffer + (offset[i] * 8), len[i] * sizeof (uint8_t));

    		// Ethernet frame length = ethernet header (MAC + MAC + ethernet type) + ethernet data (IP header + fragment)
    		frame_length = ETH_HDRLEN + IP4_HDRLEN + len[i];

    		// Send ethernet frame to socket.
#ifdef DEBUG_MSG
    		printf ("Sending fragment: %i\n", i);
#endif
			if ((bytes = sendto (sd, ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device))) <= 0) {
      			perror ("sendto() failed");
      			exit (EXIT_FAILURE);
    		}
  		}  // End loop nframes

  		// Close socket descriptor.
  		close (sd);
		
		usleep(1000000);
}
  // Free allocated memory.
  free (src_mac);
  free (dst_mac);
  free (ether_frame);
  free (interface);
  free (target);
  free (src_ip);
  free (dst_ip);
  free (ip_flags);
  free (payload);
  free (buffer);

  return (EXIT_SUCCESS);
}

// Checksum function
uint16_t
checksum (uint16_t *addr, int len)
{
  int nleft = len;
  int sum = 0;
  uint16_t *w = addr;
  uint16_t answer = 0;

  while (nleft > 1) {
    sum += *w++;
    nleft -= sizeof (uint16_t);
  }

  if (nleft == 1) {
    *(uint8_t *) (&answer) = *(uint8_t *) w;
    sum += answer;
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

// Build IPv4 UDP pseudo-header and call checksum function.
uint16_t
udp4_checksum (struct ip iphdr, struct udphdr udphdr, uint8_t *payload, int payloadlen)
{
  char buf[IP_MAXPACKET];
  char *ptr;
  int i, chksumlen = 0;

  memset (buf, 0, IP_MAXPACKET * sizeof (uint8_t));

  ptr = &buf[0];  // ptr points to beginning of buffer buf

  // Copy source IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_src.s_addr, sizeof (iphdr.ip_src.s_addr));
  ptr += sizeof (iphdr.ip_src.s_addr);
  chksumlen += sizeof (iphdr.ip_src.s_addr);

  // Copy destination IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_dst.s_addr, sizeof (iphdr.ip_dst.s_addr));
  ptr += sizeof (iphdr.ip_dst.s_addr);
  chksumlen += sizeof (iphdr.ip_dst.s_addr);

  // Copy zero field to buf (8 bits)
  *ptr = 0; ptr++;
  chksumlen += 1;

  // Copy transport layer protocol to buf (8 bits)
  memcpy (ptr, &iphdr.ip_p, sizeof (iphdr.ip_p));
  ptr += sizeof (iphdr.ip_p);
  chksumlen += sizeof (iphdr.ip_p);

  // Copy UDP length to buf (16 bits)
  memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
  ptr += sizeof (udphdr.len);
  chksumlen += sizeof (udphdr.len);

  // Copy UDP source port to buf (16 bits)
  memcpy (ptr, &udphdr.source, sizeof (udphdr.source));
  ptr += sizeof (udphdr.source);
  chksumlen += sizeof (udphdr.source);

  // Copy UDP destination port to buf (16 bits)
  memcpy (ptr, &udphdr.dest, sizeof (udphdr.dest));
  ptr += sizeof (udphdr.dest);
  chksumlen += sizeof (udphdr.dest);

  // Copy UDP length again to buf (16 bits)
  memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
  ptr += sizeof (udphdr.len);
  chksumlen += sizeof (udphdr.len);

  // Copy UDP checksum to buf (16 bits)
  // Zero, since we don't know it yet
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 2;

  // Copy payload to buf
  memcpy (ptr, payload, payloadlen);
  ptr += payloadlen;
  chksumlen += payloadlen;

  // Pad to the next 16-bit boundary
  i = 0;
  while (((payloadlen+i)%2) != 0) {
    i++;
    chksumlen++;
    ptr++;
  }

  return checksum ((uint16_t *) buf, chksumlen);
}

// Allocate memory for an array of chars.
char *
allocate_strmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (char *) malloc (len * sizeof (char));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (char));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of unsigned chars.
uint8_t *
allocate_ustrmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (uint8_t));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of ints.
int *
allocate_intmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_intmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (int *) malloc (len * sizeof (int));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (int));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_intmem().\n");
    exit (EXIT_FAILURE);
  }
}
