#!/bin/bash
[ "$UID" -eq 0 ] || exec sudo "$0" "$@"

echo "Setting up cube"
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
RPI_HW_NUM=$(cat /sys/firmware/devicetree/base/model | awk '{print $3}')

apt-get update -y
apt-get upgrade -y
apt-get install build-essential -y
apt-get install libbsd-dev -y
apt-get install nmap -y

wget https://gist.githubusercontent.com/aallan/b4bb86db86079509e6159810ae9bd3e4/raw/846ae1b646ab0f4d646af9115e47365f4118e5f6/mac-vendor.txt -P /media/


echo "Installing hostapd, dnsmasq and configuring dhcp"
apt-get install dnsmasq -y
apt-get install hostapd -y


systemctl unmask hostapd
systemctl enable hostpad


DEBIAN_FRONTEND=noninteractive apt install -y netfilter-persistent iptables-persistent
cd "$parent_path"


iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
netfilter-persistent save

mv /etc/dnsmasq.conf /etc/dnsmasq.conf.old
cp ./config/dnsmasq.conf /etc/

echo "Setting up network bridge"

cp ./config/bridge-br0.netdev /etc/systemd/network/
cp ./config/br0-member-eth0.network /etc/systemd/network/

systemctl enable systemd-networkd

echo "Configuring Access Point"

mv /etc/dhcpcd.conf /etc/dhcpcd.conf.old
cp ./config/dhcpcd.conf /etc/

rfkill unblock wlan
iw reg set US

if [ -f /etc/hostapd/hostapd.conf ]; then
    mv /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.conf.old
fi


cp ./config/hostapd.conf /etc/hostapd/hostapd.conf
cp ./config/hostapd.service /etc/systemd/system/multi-user.target.wants/

systemctl unmask hostapd
systemctl enable hostapd
systemctl enable hostapd.service
systemctl start hostapd

if [ ! -f /var/spool/cron/root ]; then
	   echo "cron file for root does not exist, creating.."
	   touch /var/spool/cron/root
	   /usr/bin/crontab /var/spool/cron/root
fi

cronjob="@reboot sudo /usr/sbin/service hostapd start"
(crontab -u root -l; echo "$cronjob" ) | crontab -u root -

echo "Done setting up cube as access point. Installing other required software...."
apt-get install libjson-c-dev -y
apt-get install sqlite3 -y
apt-get install libsqlite3-dev -y
apt-get install vim -y

echo "Creating database and tables"
cd /media

sqlite3 -batch splicecube.db "create table cubelets (cubelet_id INTEGER PRIMARY KEY AUTOINCREMENT,ip_addr TEXT,mac_addr TEXT, hostname TEXT,time_added TIMESTAMP DEFAULT CURRENT_TIMESTAMP, last_seen DATETIME, notes TEXT);"
sqlite3 -batch splicecube.db "create table wifi_traffic (id INTEGER PRIMARY KEY AUTOINCREMENT,time DATETIME,src_ip TEXT,dst_ip TEXT,src_mac_addr TEXT, dst_mac_addr TEXT, rssi INTEGER, notes TEXT);"
sqlite3 -batch splicecube.db "create table device_inventory(id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT,vendor TEXT,mac_addr TEXT, ip_addr TEXT, time_added DATETIME, last_seen DATETIME, notes TEXT);"

cd "$parent_path"

echo "Installing software.."
cd ./cube
make all && make install

cd "$parent_path"

echo "Setting up other services..."
cp ./config/recv-data.service /etc/systemd/system/
systemctl enable recv-data.service
systemctl start recv-data.service

cronjob="@reboot sudo systemctl start recv-data.service"
(crontab -u root -l; echo "$cronjob") | crontab -u root -

cp ./config/cubelet-commissioner.service /etc/systemd/system/
systemctl enable cubelet-commissioner.service
systemctl start cubelet-commissioner.service

cronjob="@reboot sudo systemctl start cubelet-commissioner.service"
(crontab -u root -l; echo "$cronjob" ) | crontab -u root -

echo "Changing hostname"
hostnamectl set-hostname cube
echo "127.0.0.1 cube" | tee -a /etc/hosts

echo "Setting up Prometheus, Pushgateway and Grafana..."
cd ~
mkdir -p /etc/prometheus/
wget https://github.com/prometheus/prometheus/releases/download/v2.22.0/prometheus-2.22.0.linux-armv7.tar.gz
tar xfz prometheus-2.22.0.linux-armv7.tar.gz
mv prometheus-2.22.0.linux-armv7/prometheus /usr/local/bin
mv prometheus-2.22.0.linux-armv7/promtool /usr/local/bin
mv prometheus-2..22.0.linux-armv7/consoles /etc/prometheus/
mv prometheus-2..22.0.linux-armv7/console_libraries /etc/prometheus/

if [ -f /etc/hostapd/hostapd.conf ]; then
    mv /etc/prometheus/prometheus.yml /etc/prometheus/prometheus.yml.old
fi

rm prometheus-2.22.0.linux-armv7.tar.gz
rm -rf prometheus-2.22.0.linux-armv7

wget https://github.com/prometheus/pushgateway/releases/download/v1.4.3/pushgateway-1.4.3.linux-armv7.tar.gz
tar -xvf pushgateway-1.4.3.linux-armv7.tar.gz
mv pushgateway-1.4.3.linux-armv7/pushgateway /usr/local/bin
rm -rf pushgateway*

apt-get install -y apt-transport-https software-properties-common
wget -q -O - https://packages.grafana.com/gpg.key | apt-key add -
echo "deb https://packages.grafana.com/oss/deb stable main" | tee -a /etc/apt/sources.list.d/grafana.list
apt-get install grafana -y

cd "$parent_path"

cp ./config/prometheus.service /etc/systemd/system/
cp ./config/prometheus.yml /etc/prometheus/
cp ./config/pushgateway.service /etc/systemd/system
cp ./cube/python/scrape_bytes.py /usr/local/sbin
cp ./config/scrape-bytes.service /etc/systemd/system/

chmod +x /usr/local/sbin/scrape_bytes.py

systemctl enable prometheus
systemctl start prometheus

systemctl enable pushgateway
systemctl start pushgateway

systemctl enable grafana-server
start grafana-server

systemctl enable scrape-bytes.service
systemctl start scrape-bytes.service

echo "Rebooting..."
systemctl reboot


