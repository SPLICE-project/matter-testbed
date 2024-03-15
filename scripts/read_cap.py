#!/usr/bin/env python3
import pyshark

def process_pcap_from_pipe(pipe_path):
    cap = pyshark.LiveCapture(interface="nrf Sniffer for 802.15.4", use_json=True)

    for packet in cap.sniff_continuously():
        print(packet)


if __name__ == "__main__":
    process_pcap_from_pipe('file.pcap')
