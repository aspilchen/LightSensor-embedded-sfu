#ifndef UDP_H
#define UDP_H

#include <stdint.h>
#include <netdb.h>

#define MSG_MAX_LEN 1500

int32_t udp_openSocket(const int32_t port);
int32_t udp_receive(const int32_t socketDescriptor, char* buffer, const uint32_t maxLen, struct sockaddr_in* sockAddr);
void udp_send(const int32_t socketDescriptor, char* buffer, struct sockaddr_in* sockAddr);
void udp_closeSocket(const int32_t socketDescriptor);

#endif