#include "rpiInfo.h"

#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/if_link.h>
#include <net/if.h>  // Added this include

/*
 * Get the IP address of the default network interface
 */
char* get_ip_address(void)
{
    struct ifaddrs *ifaddr, *ifa;
    int family;
    char *ip_address = NULL;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return strdup("xxx.xxx.xxx.xxx");
    }

    // Iterate over the linked list of interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        // Look for IPv4 addresses
        if (family == AF_INET) {
            // Exclude loopback interface
            if (strcmp(ifa->ifa_name, "lo") == 0)
                continue;

            // Check if interface is up
            if (!(ifa->ifa_flags & IFF_UP))
                continue;

            // Use this IP address
            ip_address = strdup(inet_ntoa(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr));
            break;
        }
    }

    freeifaddrs(ifaddr);

    if (ip_address != NULL)
        return ip_address;
    else
        return strdup("xxx.xxx.xxx.xxx");
}

/*
 * get RAM memory
 */
void get_cpu_memory(float *Totalram, float *freeram)
{
    struct sysinfo s_info;
    unsigned int value = 0;
    char buffer[100] = {0};
    char famer[100] = {0};

    if (sysinfo(&s_info) == 0) // Get memory information
    {
        FILE *fp = fopen("/proc/meminfo", "r");
        if (fp == NULL)
        {
            return;
        }
        while (fgets(buffer, sizeof(buffer), fp))
        {
            if (sscanf(buffer, "%s%u", famer, &value) != 2)
            {
                continue;
            }
            if (strcmp(famer, "MemTotal:") == 0)
            {
                *Totalram = value / 1024.0 / 1024.0;
            }
            else if (strcmp(famer, "MemAvailable:") == 0)
            {
                *freeram = value / 1024.0 / 1024.0;
            }
        }
        fclose(fp);
    }
}

/*
 * get SD memory
 */
void get_sd_memory(uint32_t *MemSize, uint32_t *freesize)
{
    struct statfs diskInfo;
    statfs("/", &diskInfo);
    unsigned long long blocksize = diskInfo.f_bsize;              // The number of bytes per block
    unsigned long long totalsize = blocksize * diskInfo.f_blocks; // Total number of bytes
    *MemSize = (unsigned int)(totalsize >> 30);
    unsigned long long size = blocksize * diskInfo.f_bfree; // Now let's figure out how much space we have left
    *freesize = size >> 30;
    *freesize = *MemSize - *freesize;
}

/*
 * get hard disk memory
 */
uint8_t get_hard_disk_memory(uint16_t *diskMemSize, uint16_t *useMemSize)
{
    *diskMemSize = 0;
    *useMemSize = 0;
    char diskMembuff[20] = {0};
    char useMembuff[20] = {0};
    FILE *fd = NULL;
    fd = popen("df -l / | tail -1 | awk '{printf \"%s\", $(2)}'", "r");
    fgets(diskMembuff, sizeof(diskMembuff), fd);
    pclose(fd);
    fd = popen("df -l / | tail -1 | awk '{printf \"%s\", $(3)}'", "r"); 
    fgets(useMembuff, sizeof(useMembuff), fd);
    pclose(fd);
    *diskMemSize = atoi(diskMembuff) / 1024;
    *useMemSize = atoi(useMembuff) / 1024;
    return 0;
}

/*
 * get temperature
 */
uint8_t get_temperature(void)
{
    FILE *fd;
    unsigned int temp;
    char buff[10] = {0};
    fd = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fd == NULL)
    {
        perror("Failed to read temperature");
        return 0;
    }
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%d", &temp);
    fclose(fd);
    return TEMPERATURE_TYPE == FAHRENHEIT ? temp / 1000 * 1.8 + 32 : temp / 1000;
}

/*
 * Get CPU usage
 */
uint8_t get_cpu_message(void)
{
    FILE *fp;
    char usCpuBuff[16] = {0};
    char syCpuBuff[16] = {0};
    int usCpu = 0;
    int syCpu = 0;
    fp = popen("top -bn1 | grep '%Cpu' | awk '{printf \"%.0f\", $(2)}'", "r"); // Gets the user CPU load
    if (fp == NULL)
    {
        perror("Failed to get user CPU usage");
        return 0;
    }
    fgets(usCpuBuff, sizeof(usCpuBuff), fp);
    pclose(fp);
    fp = popen("top -bn1 | grep '%Cpu' | awk '{printf \"%.0f\", $(4)}'", "r"); // Gets the system CPU load
    if (fp == NULL)
    {
        perror("Failed to get system CPU usage");
        return 0;
    }
    fgets(syCpuBuff, sizeof(syCpuBuff), fp);
    pclose(fp);
    usCpu = atoi(usCpuBuff);
    syCpu = atoi(syCpuBuff);
    return usCpu + syCpu;
}
