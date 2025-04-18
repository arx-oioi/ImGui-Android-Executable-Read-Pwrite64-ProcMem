#include <thread>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <iostream>
#include <cstring>
#include <stdint.h>
#include "Includes/obfuscate.h"
#include "Includes/Logger.h"
int handle;
typedef char PACKAGENAME[64];

long int GetAddr(int pid, const char *LibBase) {
    FILE *fp;
    long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];

    while (1) {
        snprintf(filename, sizeof(filename), OBFUSCATE("/proc/%d/maps"), pid);
        fp = fopen(filename, OBFUSCATE("r"));
        if (fp != NULL) {
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, LibBase)) {
                    pch = strtok(line, OBFUSCATE("-"));
                    addr = strtoul(pch, NULL, 16);
                    fclose(fp);
                    return addr;
                }
            }
            fclose(fp);
        }
       
        printf(OBFUSCATE("%s ยังไม่พบ รอสักครู่...\n"), LibBase);
        sleep(1);
    }
}
int getPID(const char* PackageName) {
    char command[256];
    char buffer[128];
    FILE *fp;
    int pid = -1;
    snprintf(command, sizeof(command), "pidof %s", PackageName);
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        pid = atoi(buffer);
    } else {
        printf("PACKAGENAME WAIT.....\n");
    }
    pclose(fp);
    return pid;
}
int PHEX(long int addr, const char *hex_string) {

    size_t len = strlen(hex_string);
    if (len % 2 != 0) {
        fprintf(stderr, OBFUSCATE("จำนวนของ Hex ไม่สมบูรณ์\n"));
        return -1;
    }
    size_t byte_len = len / 2;
    uint64_t value = 0;
    sscanf(hex_string, OBFUSCATE("%llx"), &value);

    if (byte_len == 4) {
        uint32_t set_endian_value = __builtin_bswap32((uint32_t)value);
        ssize_t result = pwrite64(handle, &set_endian_value, sizeof(set_endian_value), addr);
        /*
        if (result == sizeof(set_endian_value)) {

        } else {

        }*/

    } else if (byte_len == 8) {
        uint64_t set_endian_value = __builtin_bswap64(value);
        ssize_t result = pwrite64(handle, &set_endian_value, sizeof(set_endian_value), addr);
        /*
        if (result == sizeof(set_endian_value)) {

        } else {

        }*/
        
    } else {
        for (size_t i = 0; i < byte_len; i++) {
            uint8_t byte = (value >> (8 * (byte_len - 1 - i))) & 0xFF;
            ssize_t result = pwrite64(handle, &byte, sizeof(byte), addr + i);
          /*
            if (result == sizeof(byte)) {

            } else {

            }*/
            
        }
    }

    return 0;
}
int thumb(long int addr, uint16_t instruction) {
    pwrite64(handle, &instruction, 2, addr);
    return 0;
}
int PFLOAT(long int addr, float value) {
    pwrite64(handle, &value, 4, addr);
    return 0;
}
int PDWORD(long int addr, int value) {
    pwrite64(handle, &value, 4, addr);
    return 0;
}
int READMEM(int ipid){

}



