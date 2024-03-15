/*
This file helps in registering a new cubelet with the SPLICEcube.
It checks a database (SPLICEcube.db/cubelets) to see if the
cubelet was previously registered (using mac addr).

Returns a integer to cubelet. The aforementioned integer influences
the hostname for said cubelet.
*/
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sqlite3.h>
#include <arpa/inet.h>

#define PORT 8006
#define MAX_SIZE 1024
#define DB_PATH "/media/splicecube.db" //database path

int add_new_cubelet(sqlite3 *db, int cubelet_num, char *mac_addr, char *ip_addr){
    char *insert = sqlite3_mprintf("insert into cubelets (hostname, ip_addr, mac_addr) values('cubelet%d', '%q', '%q')", cubelet_num,
                                   ip_addr, mac_addr);
    if (sqlite3_exec(db, insert, NULL, NULL, NULL) != SQLITE_OK){
        return -1;
    }
    return 1;
}


int get_cubelet_num(char *database_path, char *mac_addr, char *ip_addr){
    int id;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int rc = sqlite3_open(database_path, &db);

    if (rc != SQLITE_OK){
        fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    //check if a ID number exists for mac address

    char *query = sqlite3_mprintf("select cubelet_id from cubelets where mac_addr = '%q'", mac_addr);
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL)) {
        fprintf(stderr, "Error executing sql statement...\n");
        sqlite3_close(db);
        return -1;
    }

    //if no mac_addr found
    if (sqlite3_step(stmt) == SQLITE_DONE){
        fprintf(stdout, "No previous record found for device with MAC: %s\n", mac_addr);
        //if no mac address found, find the highest number and return +1
        sqlite3_reset(stmt);

        if (sqlite3_prepare_v2(db, "select max(cubelet_id) from cubelets", -1, &stmt, NULL)) {
            fprintf(stderr, "Error executing sql statement...\n");
            sqlite3_close(db);
            return -1;
        }

        if (sqlite3_step(stmt) == SQLITE_DONE){
            fprintf(stdout, "Adding first cube.\n");
            id = 1;

        }else{
            id = sqlite3_column_int(stmt, 0);
            id = id + 1;
        }
        //no record exists for that mac address so we create a new one
        if (add_new_cubelet(db, id, mac_addr, ip_addr) < 0){
            fprintf(stderr, "Can not create new entry for cubelet with MAC addr %s\n", mac_addr);
            return -1;
        }

    }else{
        id = sqlite3_column_int(stmt, 0);
        fprintf(stdout, "Matching MAC Address found. Cubelet ID is: %d\n", id);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return id;

}



int main(void){
    char buff[MAX_SIZE];
    int sockfd, client_sock, client_len, cubelet_num;
    struct sockaddr_in servaddr, client;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1){
        fprintf(stderr, "Socket creation failed...\n");
        exit(-1);
    }

    bzero(&servaddr, sizeof(servaddr));


    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);


    if ((bind(sockfd, (struct sockaddr_in*)&servaddr, sizeof(servaddr))) !=0 ){
        fprintf(stderr, "Socket bind failed...\n");
        exit(-1);
    }

    fprintf(stdout, "Cubelet Commissioner socket created and binded.\nServer listening...\n");

    listen(sockfd, 5);

    while(1){ //keep on running

        //set up connection over socket
        client_len = sizeof(client);
        client_sock = accept(sockfd, (struct sockaddr_in*)&client, &client_len);

        if (client_sock < 0){
            fprintf(stderr, "Server accept failed.\n");
            exit(-1);
        }

        bzero(buff, sizeof(buff));

        fprintf(stdout, "Cubelet connected at IP: %s\n", inet_ntoa(client.sin_addr));

        if (recv(client_sock, buff, sizeof(buff), 0) < 0){
            fprintf(stderr, "Couldn't receive message from client.\n");
            return -1;
        }
        fprintf(stdout, "Got MAC Address: %s\n", buff);

        //actually getting the cubelet number
        cubelet_num = get_cubelet_num(DB_PATH, buff, inet_ntoa(client.sin_addr));

        bzero(buff, sizeof(buff));

        sprintf(buff, "%d", cubelet_num);

        if (send(client_sock, buff, sizeof(buff), 0) < 0){
            fprintf(stderr, "Can not reply to cubelet.\n");
            return -1;
        }

    }

    close(sockfd);
    close(client_sock);

    return 0;
}
