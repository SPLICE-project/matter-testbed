#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <endian.h>
#include <errno.h>
#include <string.h>
#include <pcap/pcap.h>
#include "include/radiotap_iter.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <json-c/json.h>
#include <time.h>
#include <sys/un.h>
#include "../../common/cube_cubelet_comm.h"

static int fcshdr = 0;

#define RATE_SOCK_PATH "/tmp/pps7"
#define PORT 8778
#define IP_ADDR "10.28.54.45"

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
struct sockaddr_in server;
struct ieee80211_hdr {
	unsigned short frame_control;
	unsigned short duration_id;
	unsigned char addr1[6];
	unsigned char addr2[6];
	unsigned char addr3[6];
	unsigned short seq_ctrl;
	unsigned short addr4[6];
} __attribute__ ((packed));

struct wifi_frame{
	unsigned char src_addr[6];
	unsigned char dest_addr[6];
};

typedef struct packet_handler_args{
    int cube_sock;
    int capture_rate_sock;
}packet_handler_args_t;

static const struct radiotap_align_size align_size_000000_00[] = {
	[0] = { .align = 1, .size = 4, },
	[52] = { .align = 1, .size = 4, },
};

static const struct ieee80211_radiotap_namespace vns_array[] = {
	{
		.oui = 0x000000,
		.subns = 0,
		.n_bits = sizeof(align_size_000000_00),
		.align_size = align_size_000000_00,
	},
};

static const struct ieee80211_radiotap_vendor_namespaces vns = {
	.ns = vns_array,
	.n_ns = sizeof(vns_array)/sizeof(vns_array[0]),
};

static void print_radiotap_namespace(struct ieee80211_radiotap_iterator *iter)
{
	switch (iter->this_arg_index) {
	case IEEE80211_RADIOTAP_TSFT:
		printf("\tTSFT: %llu\n", le64toh(*(unsigned long long *)iter->this_arg));
		break;
	case IEEE80211_RADIOTAP_FLAGS:
		printf("\tflags: %02x\n", *iter->this_arg);
		break;
	case IEEE80211_RADIOTAP_RATE:
		printf("\trate: %lf\n", (double)*iter->this_arg/2);
		break;
	case IEEE80211_RADIOTAP_CHANNEL:
		printf("\tchannel frequency: %d\n", (u_int16_t)*iter->this_arg);
		break;
	case IEEE80211_RADIOTAP_FHSS:
	case IEEE80211_RADIOTAP_DBM_ANTSIGNAL:
		printf("\tdbm antsignal: %d\n", (int8_t)*iter->this_arg);
		break;
	case IEEE80211_RADIOTAP_DBM_ANTNOISE:
		printf("\tdbm antnoise: %d\n", (int8_t)*iter->this_arg);
		break;
	case IEEE80211_RADIOTAP_LOCK_QUALITY:
	case IEEE80211_RADIOTAP_TX_ATTENUATION:
	case IEEE80211_RADIOTAP_DB_TX_ATTENUATION:
	case IEEE80211_RADIOTAP_DBM_TX_POWER:
	case IEEE80211_RADIOTAP_ANTENNA:
	case IEEE80211_RADIOTAP_DB_ANTSIGNAL:
		printf("\tdb antsignal: %d\n", (int8_t)*iter->this_arg);
		break;
	case IEEE80211_RADIOTAP_DB_ANTNOISE:
		printf("\tdb antnoise: %d\n", (int8_t)*iter->this_arg);
		break;
	case IEEE80211_RADIOTAP_TX_FLAGS:
		break;
	case IEEE80211_RADIOTAP_RX_FLAGS:
		if (fcshdr) {
			printf("\tFCS in header: %.8x\n",
				le32toh(*(uint32_t *)iter->this_arg));
			break;
		}
		printf("\tRX flags: %#.4x\n",
			le16toh(*(uint16_t *)iter->this_arg));
		break;
	case IEEE80211_RADIOTAP_RTS_RETRIES:
	case IEEE80211_RADIOTAP_DATA_RETRIES:
		break;
		break;
	default:
		printf("\tBOGUS DATA\n");
		break;
	}
}

static void print_test_namespace(struct ieee80211_radiotap_iterator *iter){
	switch (iter->this_arg_index) {
	case 0:
	case 52:
		printf("\t00:00:00-00|%d: %.2x/%.2x/%.2x/%.2x\n",
			iter->this_arg_index,
			*iter->this_arg, *(iter->this_arg + 1),
			*(iter->this_arg + 2), *(iter->this_arg + 3));
		break;
	default:
		printf("\tBOGUS DATA - vendor ns %d\n", iter->this_arg_index);
		break;
	}
}


static const struct radiotap_override overrides[] = {
	{ .field = 14, .align = 4, .size = 4, }
};

//callback for pcap_handle
void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
    int err;
    int i;
	struct ieee80211_radiotap_iterator iter;
    packet_handler_args_t *fd_args = (packet_handler_args_t*)args;

    //stuff to keep track of number of packets captured per second
    static struct timeval t1 = {0, 0};
    struct timeval t2;
    static int packet_count = 0;
    float time_elapsed, packets_per_sec;
    char float_buff[sizeof(float)];


    printf("\nGot one packet:\n");

    err = ieee80211_radiotap_iterator_init(&iter, packet, 2014, &vns);
	if (err) {
		printf("malformed radiotap header (init returns %d)\n", err);
		return;
	}

     //Parsing captured data packet and print radiotap information.
    while (!(err = ieee80211_radiotap_iterator_next(&iter))) {
		if (iter.this_arg_index == IEEE80211_RADIOTAP_VENDOR_NAMESPACE) {
			printf("\tvendor NS (%.2x-%.2x-%.2x:%d, %d bytes)\n",
				iter.this_arg[0], iter.this_arg[1],
				iter.this_arg[2], iter.this_arg[3],
				iter.this_arg_size - 6);
			for (i = 6; i < iter.this_arg_size; i++) {
				if (i % 8 == 6)
					printf("\t\t");
				else
					printf(" ");
				printf("%.2x", iter.this_arg[i]);
			}
			printf("\n");
		} else if (iter.is_radiotap_ns)
			print_radiotap_namespace(&iter);
		else if (iter.current_namespace == &vns_array[0])
			print_test_namespace(&iter);
	}

	if (err != -ENOENT) {
		printf("malformed radiotap data\n");
		return;
	}

    //calculate packets per second
    packet_count++;
    if (packet_count == 1){
        gettimeofday(&t1, NULL);
    } else {
        gettimeofday(&t2, NULL);
        time_elapsed = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
        packets_per_sec = packet_count / time_elapsed;
    }

    fprintf(stdout, "PPS %f\n", packets_per_sec);

    //get mac addresses from header
    struct ieee80211_hdr *hdr =(struct ieee80211_hdr *)(packet + iter._max_length);
    printf("\tsource address: "MACSTR"\n", MAC2STR(hdr->addr2));
    printf("\tdestination address: "MACSTR"\n", MAC2STR(hdr->addr1));
    printf("\tbssid: "MACSTR"\n", MAC2STR(hdr->addr3));

    //Place address in sniffed packet typedef
    //send to cube
    sniffed_packet_t sniffed_packet;

    sniffed_packet.cubelet_num = 1;
    sniffed_packet.rssi = 0;
    strcpy(sniffed_packet.datetime, "NULL");
    snprintf(sniffed_packet.src_addr, 18, MACSTR, MAC2STR(hdr->addr2));
    snprintf(sniffed_packet.dst_addr, 18, MACSTR, MAC2STR(hdr->addr1));

    ssize_t bytes_sent = sendto(fd_args->cube_sock, &sniffed_packet, sizeof(sniffed_packet_t), 0, (const struct sockaddr*)&server,
	sizeof(server));

    if (bytes_sent < 0){
	    fprintf(stderr, "Error sending captured data to cube.\n");
    }

    //send capture rate to pipe
    if (send(fd_args->capture_rate_sock, &packets_per_sec, sizeof(packets_per_sec), 0) < 0){
        fprintf(stderr, "Unable to send packet capture rate over socket.\n");
    }
}

int main(int argc, char *argv[]){
    pcap_t *handle;			/* Session handle */
    char *dev = "wlan1";			/* The device to sniff on */
    char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
    struct bpf_program fp;		/* The compiled filter */

	//Socket to send data to cube
	int sock;/* Socket descriptor */

	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		fprintf(stderr, "Socket creation failed.\n");
	} else {
        fprintf(stdout, "Sending socket created.\n");
    }

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP_ADDR);
	server.sin_port = htons(PORT);

	//Socket to send data to Pushgateway
    int rate_sockfd;
    struct sockaddr_un addr;
    if ((rate_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Error making UNIX domain socket.\n");
    }

    bzero(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, RATE_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(rate_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        fprintf(stderr, "Can not connect to UNIX domain socket.\n");
    }


    //pcap stuff
	handle = pcap_create(dev, errbuf);
	pcap_set_promisc(handle, 0);
	pcap_set_snaplen(handle, 2048);
	pcap_set_timeout(handle, 512);
	int status = pcap_activate(handle);
	fprintf(stdout, "Status is: %d\n", status);
	pcap_perror(handle, "%s\n");

    packet_handler_args_t fd_args = {sock, rate_sockfd};

    /* now we can set our callback function */
	pcap_loop(handle, -1, got_packet, (u_char *)&fd_args);

	/* cleanup */
	pcap_freecode(&fp);
	pcap_close(handle);
    close(rate_sockfd);
    close(sock);
    
    return(0);
}
