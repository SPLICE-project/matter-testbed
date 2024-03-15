//
// Created by ravindra on 2/19/23.
//

#ifndef CODE_CUBE_CUBELET_COMM_H
#define CODE_CUBE_CUBELET_COMM_H

typedef struct sniffed_packet {
    char datetime[12];
    char src_addr[18];
    char dst_addr[18];
    int cubelet_num;
    int rssi;
}sniffed_packet_t;

typedef struct sniffed_packets{
    struct sniffed_packet sniffed_packet[4];
}sniffed_packets_t;

#endif //CODE_CUBE_CUBELET_COMM_H
