#include "stdio.h"
#include "sys/socket.h"
#include "errno.h"
#include "netdb.h"
#include "string.h"
#include "stdlib.h"
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <json-c/json.h>

#define PORT 8111
#define MAX_SIZE 1024

int *tcp_portscan(char * ip_addr, int start_port, int end_port){

    fprintf(stdout, "Starting %s scan from port %d to %d.\n", ip_addr, start_port, end_port);

    struct hostent *host_device;
    struct sockaddr_in addr;
    int err, sock;

    strncpy((char*)&addr, "", sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);

    for (int i = start_port; i <= end_port; i++){
        addr.sin_port = htons(i);
        sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock < 0){
            fprintf(stderr, "Error creating Socket.");
            exit(1);
        }

        err = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

        if (err < 0){
            fflush(stdout);
        }else{
            //add to int array
            fprintf(stdout, "%-5d open\n", i);
        }
        close(sock);
    }

}


int *udp_portscan(char *ip_addr, int start_port, int end_port){

    fprintf(stdout, "Starting %s scan from port %d to %d.\n", ip_addr, start_port, end_port);

    struct hostent *host_device;
    struct sockaddr_in addr;
    int err, sock, icmp_sock;
    fd_set read_fd_set, write_fd_set;
    struct servent* srvport;
    char buf[1];

    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        fprintf(stderr, "IPPROTO_UDP socket failed.\n");
        exit(-1);
    }

    if ((icmp_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
        fprintf(stderr, "IPPROTO_ICMP socket failed.\n");
       	exit(-1);
    }

    struct timeval poll;
    poll.tv_sec = 1;
    poll.tv_usec = 0;

    if((host_device = gethostbyname(ip_addr)) == NULL){
        fprintf(stderr, "gethostbyname() failed.\n");
        exit(-1);
     }

    for (int i = start_port; i <= end_port; i++){
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(i);
        addr.sin_addr = *((struct in_addr *)host_device->h_addr);

        if (sendto(sock, buf, sizeof(buf), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0){
            fprintf(stderr, "sendto() failed.\n");
        }
            FD_ZERO(&write_fd_set);
            FD_SET(sock, &write_fd_set);

            if (select(sock + 1, &write_fd_set, NULL, NULL, &poll) > 0){
                recvfrom(sock, &buf, sizeof(buf), 0x0, NULL, NULL);
            }else if (!FD_ISSET(sock, &write_fd_set)){
                srvport = getservbyport(htons(i), "udp");
                if (srvport != NULL)
                printf("tport %d: %sn", i, srvport->s_name);
                fflush(stdout);
            }else{
                fprintf(stderr, "recvfrom() failed.\n");
            }

            struct ip *iphdr= (struct ip *)buf;
            int iplen = iphdr->ip_hl << 2;

            struct icmp *icmp= (struct icmp *)(buf + iplen);

            if((icmp->icmp_type == ICMP_UNREACH) && (icmp->icmp_code == ICMP_UNREACH_PORT)){
                return 0;
            }
    }
    close(sock);

}


int main(void){
    char buff[MAX_SIZE];
    int sockfd, client_sock, client_len;
    struct sockaddr_in servaddr, client;
    struct json_object *jobj, *protocol, *start_port, *end_port, *ip_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
        fprintf(stderr, "Socket creation failed...\n");
        exit(-1);
    }

    bzero(&servaddr, sizeof(servaddr));


    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);


    if ((bind(sockfd, (struct sockaddr_in*)&servaddr, sizeof(servaddr))) !=0 ){
        fprintf(stderr, "Socket bind failed...\n");
        exit(-1);
    }

    fprintf(stdout, "Port Scan socket created and binded.\nServer listening...\n");

    listen(sockfd, 5);

    while(1){
        client_len = sizeof(client);
        client_sock = accept(sockfd, (struct sockaddr_in*)&client, &client_len);

        if (client_sock < 0){
            fprintf(stderr, "Server accept failed.\n");
            exit(-1);
        }

        bzero(buff, sizeof(buff));

        fprintf(stdout, "Client connected at IP: %s and port: %i\n",
                inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        if (recv(client_sock, buff, sizeof(buff), 0) < 0){
            fprintf(stderr, "Couldn't receive message from client.\n");
            return -1;
        }

        jobj = json_tokener_parse(buff);
        json_object_object_get_ex(jobj, "Protocol", &protocol);
        json_object_object_get_ex(jobj, "IP Address", &ip_addr);
        json_object_object_get_ex(jobj, "Start Port", &start_port);
        json_object_object_get_ex(jobj, "End Port", &end_port);

        const char *proto = json_object_get_string(protocol);
        char *ip          = (char*)json_object_get_string(ip_addr);
        int s_port        = json_object_get_int(start_port);
        int e_port        = json_object_get_int(end_port);

        if (strcmp("\"TCP\"", proto) == 0){
            tcp_portscan(ip, s_port, e_port);
        }else{
            udp_portscan(ip, s_port, e_port);
        }
    }
    return 0;
}
