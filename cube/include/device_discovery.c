#define  _GNU_SOURCE
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <strings.h>
#include <sys/socket.h>
#include <ctype.h>

#include "device_discovery.h"

#define OUI_PATH "/media/mac-vendor.txt"

int get_device_name(const char *ip_addr, char *device_name){
    struct hostent *device;
    struct in_addr ip;

    if (!inet_aton(ip_addr, &ip)){
        fprintf(stderr, "Error in converting IP to binary.\n");
        return -1;
    }

    device = gethostbyaddr(&ip, sizeof(ip), AF_INET);
    if (device == NULL){
        fprintf(stderr, "Error in resolving host name for IP : %s\n", ip_addr);
        return -1;
    }

    strcpy(device_name, device->h_name);

    return 0;
}

int get_device_vendor(const char *mac_addr, char *vendor){
    int found = 0;
    char oui[8];
    char line[200];
    char vendor_info[50];
    FILE *fp;

    char mac_no_colon[18];
    int i, j = 0;
    for (i = 0; i < strlen(mac_addr); i++) {
        if (mac_addr[i] != ':') {
            mac_no_colon[j++] = mac_addr[i];
        }
    }

    strncpy(oui, mac_no_colon, 6);
    oui[6] = '\0';

    for (i = 0; i < strlen(oui); i++) {
        if (isalpha(oui[i])){
            oui[i] = toupper(oui[i]);
        }
    }

    fp = fopen(OUI_PATH, "r");
    if (fp == NULL){
        fprintf(stderr, "Error opening OUI file.\n");
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        char oui_from_file[6];
        char vendor_from_file[100];

        if (sscanf(line, "%s\t%[^\n]", oui_from_file, vendor_from_file) == 2) {
            fprintf(stdout, "%s\t%s\n", oui, oui_from_file);
            if (strcmp(oui, oui_from_file) == 0) {
                strcpy(vendor_info, vendor_from_file);
                found = 1;
                break;
            }
        }
    }

    if (found){
        strcpy(vendor, vendor_info);
        return 1;
    } else {
        fprintf(stderr, "Unable to find vendor for MAC: %s\n", mac_addr);
        return -1;
    }

}


int get_device_ip(const char* mac_addr, char* ip) {
    FILE* arp_file;
    char line[100], buf[20];
    char target_mac[18];

    for (int i = 0; i < 17; i++) {
        if (mac_addr[i] == ':') {
            target_mac[i] = ':';
        } else {
            target_mac[i] = tolower(mac_addr[i]);
        }
    }
    target_mac[17] = '\0';

    arp_file = fopen("/proc/net/arp", "r");
    if (arp_file == NULL) {
        fprintf(stderr, "Error reading arp proc.\n");
        return -1;
    }

    fgets(line, sizeof(line), arp_file);
    while (fgets(line, sizeof(line), arp_file)) {
        sscanf(line, "%s %*s %*s %s", ip, buf);
        if (strcmp(buf, target_mac) == 0) {
            fclose(arp_file);
            return 1;
        }
    }

    fclose(arp_file);
    return -1;
}
