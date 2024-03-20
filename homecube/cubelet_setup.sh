#!/bin/bash
[ "$UID" -eq 0 ] || exec sudo "$0" "$@"

CUBE_HOSTNAME="cube"
WIFI_INTERFACE="wlan0"


echo "Setting up cubelet"

if [ ! -f /var/spool/cron/root ]; then
	   echo "cron file for root does not exist, creating.."
	   touch /var/spool/cron/root
	   /usr/bin/crontab /var/spool/cron/root
fi

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

apt-get update -y
apt-get upgrade -y

apt-get install libpcap-dev -y
apt-get install libjson-c-dev -y
apt-get install netcat -y
apt-get install build-essential -y
apt-get install arp-scan -y
apt-get install nmap -y

cd "$parent_path"

mv /etc/wpa_supplicant/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant.conf.old
cp ./config/wpa_supplicant-wlan0.conf /etc/wpa_supplicant/
cp ./config/wpa_supplicant-wlan1.conf /etc/wpa_supplicant/

wpa_cli -i wlan0 reconfigure

echo "Compiling and installing software..."


cd cubelet
make all && make install

cd ..

echo "Setting up wifi sniffer"
cp ./config/sniff-wifi.service /etc/systemd/system/
cp ./cubelet/srcipts/monitor.sh /usr/local/sbin/
systemctl enable sniff-wifi.service
systemctl start sniff-wifi.service

cronjob="@reboot sudo systemctl start sniff-wifi.service"
(crontab -u root -l; echo "$cronjob" ) | crontab -u root -


echo "Setting up port scan service"
cp ./config/port-scan.service /etc/systemd/system/
systemctl enable port-scan.service
systemctl start port-scan.service

cronjob="@reboot sudo systemctl start port-scan.service"
(crontab -u root -l; echo "$cronjob" ) | crontab -u root -

echo "Registering cubelet with cube..."
CUBELET_IP=$(ip -4 addr show $WIFI_INTERFACE | grep -oP '(?<=inet\s)\d+(\.\d+){3}')
CUBE_IP=$(nmap -sP -PI -PT $CUBELET_IP/24 | grep $CUBE_HOSTNAME | cut -d "(" -f2 | cut -d ")" -f1)
echo "export CUBE_IP='$CUBE_IP'" | tee -a /etc/environment


source /etc/environment
MAC_ADDR=$(cat /sys/class/net/$WIFI_INTERFACE/address)
CUBELET_NUM=$(echo $MAC_ADDR | netcat $CUBE_IP 8006 -w3)
hostnamectl set-hostname cubelet$CUBELET_NUM
echo "127.0.0.1 cubelet$CUBELET_NUM" | tee -a /etc/hosts


cronjob="@reboot sudo systemctl set-environment CUBE_IP=$CUBE_IP"
(crontab -u root -l; echo "$cronjob" ) | crontab -u root -

echo "Setting up LED Service"
cp ./config/led.service /etc/systemd/system/
cp ./cubelet/srcipts/led.py /usr/local/sbin/
systemctl enable led.service
systemctl start led.service

cronjob="@reboot sudo systemctl start led.service"
(crontab -u root -l; echo "$cronjob" ) | crontab -u root -


echo "Installing wireshark"
sudo apt-get install wireshark -y
echo "wireshark-common wireshark-common/install-setuid boolean true" | sudo debconf-set-selections
sudo DEBIAN_FRONTEND=noninteractive dpkg-reconfigure wireshark-common

sudo usermod -a -G wireshark $USER
sudo usermod -a -G dialout $USER


echo "Rebooting..."
systemctl reboot


