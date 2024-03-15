//
// Created by ravindra on 2/19/23.
//

#include "cube_cubelet_comm.h"

struct sniffed_packet {
    char *datetime;
    char *src_addr;
    char *dst_addr;
    int  cubelet_num;
    int  rssi;
};

struct sniffed_packets {
    struct sniffed_packet sniffed_packet[4];
};
