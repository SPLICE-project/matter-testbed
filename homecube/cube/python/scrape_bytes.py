#!/usr/bin/env python3
import time
from prometheus_client import CollectorRegistry, Gauge, push_to_gateway
from threading import Thread
import subprocess
import re
import random


def get_client_info(interface="wlan0"):
    client_statistics = subprocess.check_output("iw dev " + interface + " station dump",
                                                shell=True)
    return client_statistics.decode("utf-8").split("Station")[1:]


def get_rx_tx_bytes(mac, clients=None):
    if clients is None:
        clients = get_client_info()
    client = filter(lambda x: mac in x, clients)
    client = list(client)[0]
    rx = int(re.search(r'rx bytes:\s*(\d+)', client).group(1))
    tx = int(re.search(r'tx bytes:\s*(\d+)', client).group(1))
    rx_rate = int(re.search(r'rx bitrate:\s*(\d+)', client).group(1).split(" ")[0])
    tx_rate = int(re.search(r'tx bitrate:\s*(\d+)', client).group(1).split(" ")[0])
    return rx, tx, rx_rate, tx_rate


def get_mac(client):
    return client.split(" ")[1]


def push_data(registry, client):
    mac = get_mac(client)
    print("Starting push_data for: ", mac)
    rx_gauge = Gauge("RX_Bytes_" + mac, "Received Bytes", registry=registry)
    tx_gauge = Gauge("TX_Bytes_" + mac, "Transmitted Bytes", registry=registry)
    rx_rate_gauge = Gauge("RX_Rate_" + mac, "Received MBit/s", registry=registry)
    tx_rate_gauge = Gauge("TX_Rate_" + mac, "Transmitted MBit/s", registry=registry)
    while True:
        rx, tx, rx_rate, tx_rate = get_rx_tx_bytes(mac=mac)
        rx_gauge.set(rx)
        tx_gauge.set(tx)
        rx_rate_gauge.set(rx_rate)
        tx_rate_gauge.set(tx_rate)
        push_to_gateway("http://localhost:9091", job="connected_clients", registry=registry)
        time.sleep(3)


def main():
    clients = get_client_info()
    registry = CollectorRegistry()

    threads = []

    for client in clients:
        thread = Thread(target=push_data, args=[registry, client])
        thread.start()
        threads.append(thread)

    for thread in threads:
        thread.join()


if __name__ == "__main__":
    main()
