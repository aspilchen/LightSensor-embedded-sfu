#include <string.h>
#include <unistd.h>			// for close()

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>	

#include <stdio.h>
#include "udp.h"

static int32_t udp_openSocketImpl(const int32_t port);
static int32_t udp_receiveImpl(const int32_t socketDescriptor, char* buffer, const uint32_t maxLen, struct sockaddr_in* sockAddr);
static void udp_sendImpl(const int32_t socketDescriptor, char* buffer, struct sockaddr_in* sockAddr);
static void udp_closeSocketImpl(const int32_t socketDescriptor);

int32_t udp_openSocket(const int32_t port)
{
	return udp_openSocketImpl(port);
}

int32_t udp_receive(const int32_t socketDescriptor, char* buffer, const uint32_t maxLen, struct sockaddr_in* sockAddr)
{
	return udp_receiveImpl(socketDescriptor, buffer, maxLen, sockAddr);
}

void udp_send(const int32_t socketDescriptor, char* buffer, struct sockaddr_in* sockAddr)
{
	udp_sendImpl(socketDescriptor, buffer, sockAddr);
}

void udp_closeSocket(const int32_t socketDescriptor)
{
	udp_closeSocketImpl(socketDescriptor);
}

int32_t udp_openSocketImpl(const int32_t port)
{
	int32_t socketDescriptor;
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;                   // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network long
	sin.sin_port = htons(port);                 // Host to Network short
	socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
	bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));	
	return socketDescriptor;
}

int32_t udp_receiveImpl(const int32_t socketDescriptor, char* buffer, const uint32_t maxLen, struct sockaddr_in* sockAddr)
{
	int32_t bytesRead;
	uint32_t sinLen = sizeof(*sockAddr);
	bytesRead = recvfrom(socketDescriptor,
						 buffer, maxLen-1, 0,
						 (struct sockaddr *)sockAddr,
						 &sinLen);
	if (bytesRead > 1) {
		buffer[bytesRead-1] = '\0'; // remove newline char
	} else {
		buffer[bytesRead] = '\0';
	}
	return bytesRead;
}

void udp_sendImpl(const int32_t socketDescriptor, char* buffer, struct sockaddr_in* sockAddr)
{
	sendto(socketDescriptor,
		   buffer, strlen(buffer),
		   0,
		   (struct sockaddr *)sockAddr, sizeof(*sockAddr));
}

void udp_closeSocketImpl(const int32_t socketDescriptor)
{
	close(socketDescriptor);
}