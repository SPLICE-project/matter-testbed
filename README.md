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

For convenince we provide two scripts to assist with the setup of the hub system. There are two scripts which help in setting up the cube and cubelets.
The setup script for the cube creates the Access Point for the cube whilst the cubelet setup script helps in configuring the hardware and software required to do traffic capture. 
The setup script for the cubelet can be found [here](https://github.com/SPLICE-project/matter-testbed/blob/main/homecube/cubelet_setup.sh) and the cube script can be found [here](https://github.com/SPLICE-project/matter-testbed/blob/main/homecube/cube_setup.sh).

## Home Assistant Server
For the smart home platform we use [Home Assistant](https://www.home-assistant.io/). Specifically, we use a version called Home Assistant Supervised running on a Linux/Debian host
operating system. The setup instructions can be found [here](https://community.home-assistant.io/t/guide-installing-ha-supervised-on-debian-official-distro/555228).

## Matter Devices
