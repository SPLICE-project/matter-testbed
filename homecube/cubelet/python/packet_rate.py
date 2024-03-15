#!/usr/bin/env python3
import socket
import struct
from prometheus_client import CollectorRegistry, Gauge, push_to_gateway

CUBE_NUM = "1"

RATE_SOCK_PATH = "/tmp/pps7"
CUBE_PUSHGATEWAY = "10.28.54.45:9091"
JOB = "packet_capturer"
GAUGE_NAME = JOB + "_cubelet_" + CUBE_NUM

def main():
    #Socket to receive data from sniffer
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.bind(RATE_SOCK_PATH)
    sock.listen(1)

    print("Ready...")
    conn, addr = sock.accept()

    #set up gauge
    registry = CollectorRegistry()
    rate_gauge = Gauge(GAUGE_NAME, "Packets Captured / Second", registry=registry)

    while True:
        data = conn.recv(1024)
        try:
            value = struct.unpack('f', data)[0]
            rate_gauge.set(value)
            push_to_gateway(CUBE_PUSHGATEWAY, job=JOB, registry=registry)
            print("Value: ", value)
        except struct.error as err:
            print(err)

    conn.close()
    sock.close()


if __name__ == "__main__":
    main()