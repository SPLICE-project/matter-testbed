#ifndef __DEVICE_DISCOVERY_H
#define __DEVICE_DISCOVERY_H

#include <stdio.h>
#include <stdlib.h>

//Get the hostname
int get_device_name(const char *ip_addr, char *device_name);
//Get the vendor of the device
int get_device_vendor(const char *mac_addr, char *vendor);
//Get the ip of the device
int get_device_ip(const char *mac_addr, char *ip);


#endif
