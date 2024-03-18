# Matter Testbed Setup
This repository guides through the setup of a testbed that is meant to capture traffic from devices using the Matter
protocol. Both Thread and Wi-Fi data are captured. The testbed comprises of three primary components, namely:
1. **The Hub System**
2. **Home Assistant Server**
3. **Matter Devices**

## The Hub System
The hub consists of two single board computers, in this case two Raspberry Pis v4. One Raspberry Pi acts as a cube and another acts as a cubelet. 
The cube acts as the main Wi-Fi access point for the network whilst the cubelet houses the necessary hardware to sniff Wi-Fi, Thread and Bluetooth Low Energy (BLE) traffic. To be able to capture traffic, the cubelet requires the following hardware:
* nRF52840 for Thread Traffic Capture
* nRF52840 for BLE Traffic Capture
* Wi-Fi interface capable of monitor mode for Wi-Fi capture




## Home Assistant Server

## Matter Devices
