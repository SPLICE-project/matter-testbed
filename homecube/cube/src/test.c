//
// Created by ravindra on 2/27/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/device_discovery.h"

int main(int argc, char *argv[]){
    printf("%s\n", argv[1]);
    char buff[30];
    char name[20];
    strcpy(name, argv[1]);
    fprintf(stdout,"Checking for: %s\n", name);
    int a = get_device_vendor(argv[1], &buff);
    fprintf(stdout, "Ans: %s\n", buff);


    return 0;
}
