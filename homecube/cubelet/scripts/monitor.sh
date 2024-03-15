#!/usr/bin/env bash

#This scripts sets the wifi interface to monitor mode

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root. Exiting..."
   exit 1
fi

ip link set wlan1 down
iw wlan1 set monitor none
ip link set wlan1 up

