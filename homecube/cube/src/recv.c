/*
 * This file creates a socket to receive sniffed traffic data from the cubelets.
 * The data then gets stored in a database.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sqlite3.h>
#include "../../common/cube_cubelet_comm.h"

#define MAX_SIZE 1024

#define PORT 8778
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#define DB_PATH "/media/splicecube.db" //database path

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server;
    struct sockaddr_in client;
    unsigned int client_length;
    sqlite3 *db;
    char *err_msg = 0;
    char *sql_insert = (char *)malloc(30 * sizeof(char)) ;


    //open database
    int rc = sqlite3_open(DB_PATH, &db);

    if (rc != SQLITE_OK){
        fprintf(stderr, "Cannot open database %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return -1;
    }

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        fprintf(stderr, "Socket creation failed.\n");
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
        fprintf(stderr, "Bind failed.\n");
    }

    fprintf(stdout, "Ready to receive data...\n");

    sniffed_packet_t sniffed_packet;

    while (1) { //keep on receiving
	    client_length = sizeof(client);
        bzero(&sniffed_packet, sizeof(sniffed_packet_t));
        recvfrom(sock, &sniffed_packet, sizeof(sniffed_packet_t), 0, (struct sockaddr *)&client, &client_length);

        fprintf(stdout, "SRC: %s \t DST: %s\n", sniffed_packet.src_addr, sniffed_packet.dst_addr);

        sprintf(sql_insert, "insert into wifi_traffic (%s, %s);", sniffed_packet.dst_addr,
                sniffed_packet.src_addr);

        rc = sqlite3_exec(db, sql_insert, 0, 0, &err_msg);

        if (rc != SQLITE_OK ) {

            fprintf(stderr, "SQL error: %s\n", err_msg);

            sqlite3_free(err_msg);
            sqlite3_close(db);

            return 1;
        }

    }

    sqlite3_close(db);
    close(sock);
    return 0;
}
