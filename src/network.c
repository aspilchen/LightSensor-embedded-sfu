#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <stdbool.h>
#include <pthread.h>

#include "udp.h"
#include "network.h"
#include "main.h"
#include "light_sampler.h"

struct network_CmdArg
{
	struct sockaddr_in* addr;
	char* args;
};

#define NETWORK_MAX_COMMAND_SIZE 32
struct network_Command
{
	char symbol[NETWORK_MAX_COMMAND_SIZE];
	char name[NETWORK_MAX_COMMAND_SIZE];
	char* description;
	void (*cmdAction)(struct network_CmdArg* arg);
};

static void network_countFunc(struct network_CmdArg* arg);
static void network_lengthFunc(struct network_CmdArg* arg);
static void network_historyFunc(struct network_CmdArg* arg);
static void network_getFunc(struct network_CmdArg* arg);
static void network_dipsFunc(struct network_CmdArg* arg);
static void network_stopFunc(struct network_CmdArg* arg);
static void network_helpFunc(struct network_CmdArg* arg);
static void network_defaultFunc(struct network_CmdArg* arg);
static void network_printArray(struct network_CmdArg* arg, lightsample_t* values, const samplesize_t size);

#define NETWORK_N_CMDS 8
#define NETWORK_REPEAT_CMD 6
#define NETWORK_DEFAULT_CMD NETWORK_N_CMDS + 1
static struct network_Command commands[NETWORK_N_CMDS+1] = // +1 to include default error handling
{
	// symbol    display   description                                                      action function
	 {"count"   ,"count"   ,"display total number of samples taken."                        ,network_countFunc}
	,{"length"  ,"length"  ,"display number of samples in history (both max, and current)." ,network_lengthFunc}
	,{"history" ,"history" ,"display the full sample history being saved."                  ,network_historyFunc}
	,{"get"     ,"get N"   ,"display the N most recent history values."                     ,network_getFunc}
	,{"dips"    ,"dips"    ,"display the number of dips."                                   ,network_dipsFunc}
	,{"stop"    ,"stop"    ,"cause the server program to end."                              ,network_stopFunc}
	,{"\n"      ,"<enter>" ,"repeat last command."                                          ,network_defaultFunc}
	,{"help"    ,"help"    ,"display command menu."                                         ,network_helpFunc}
	,{""        ,"default" ,"display error message."                                        ,network_defaultFunc}
};


static pthread_t thread;
static bool isRunning = true;
static int32_t socketDescriptor;
static char buffer[MSG_MAX_LEN];

static int32_t network_parseCommand(char* buffer);
static inline void network_act(int32_t cmdNum, struct network_CmdArg* arg);
static void* network_threadFunc(void* arg);

void network_init(void)
{
	isRunning = true;
	pthread_create(&thread, NULL, network_threadFunc, NULL);
}

void network_cleanup(void)
{
	isRunning = false;
	pthread_cancel(thread);
}

static void* network_threadFunc(void* arg)
{
	static bool initialized = false;
	const int32_t port = 12345;
	int32_t bytesRead;
	int32_t cmdNumber;
	struct sockaddr_in sockAddr;
	struct network_CmdArg cmdArg;
	if (!initialized) {
		socketDescriptor = udp_openSocket(port);
		initialized = true;
	}
	while (isRunning) {
		bytesRead = udp_receive(socketDescriptor, buffer, MSG_MAX_LEN, &sockAddr);
		cmdArg.addr = &sockAddr;
		if (bytesRead < 0) {
			network_act(NETWORK_DEFAULT_CMD, &cmdArg);
		}
		cmdNumber = network_parseCommand(buffer);
		cmdArg.args = buffer;
		network_act(cmdNumber, &cmdArg);
	}
	close(port);
	return NULL;
}

static int32_t network_parseCommand(char* buff)
{
	int32_t cmdNumber = 0;
	buff = strtok(buff, " ");
	for (cmdNumber = 0; cmdNumber < NETWORK_N_CMDS; cmdNumber++) {
		if (strcmp(commands[cmdNumber].symbol, buff) == 0) {
			break;
		}
	}
	buff = strtok(NULL, " ");
	return cmdNumber;
}

static inline void network_act(int32_t cmdNum, struct network_CmdArg* arg)
{
	commands[NETWORK_REPEAT_CMD].cmdAction = commands[cmdNum].cmdAction;
	commands[cmdNum].cmdAction(arg);
}

static void network_countFunc(struct network_CmdArg* arg)
{
	samplesize_t count = sampler_count();
	snprintf(buffer, MSG_MAX_LEN-1, "Number of samples taken = %d.\n", count);
	udp_send(socketDescriptor, buffer, arg->addr);
}

static void network_lengthFunc(struct network_CmdArg* arg)
{
	samplesize_t capacity = sampler_capacity();
	samplesize_t size = sampler_size();
	snprintf(buffer, MSG_MAX_LEN
			,"History can hold %d samples.\nCurrently holding %d samples.\n"
			,capacity, size);
	udp_send(socketDescriptor, buffer, arg->addr);
}

static void network_historyFunc(struct network_CmdArg* arg)
{	
	lightsample_t samples[SAMPLER_MAX_HISTORY];
	samplesize_t sampleSize = sampler_history(samples, MSG_MAX_LEN);
	network_printArray(arg, samples, sampleSize);
}

static void network_getFunc(struct network_CmdArg* arg)
{
	lightsample_t samples[SAMPLER_MAX_HISTORY];
	uint32_t size = atoi(buffer);
	samplesize_t sampleSize = sampler_history(samples, SAMPLER_MAX_HISTORY);
	int32_t start = sampleSize - size - 1;
	if (start < 0) {
		start = 0;
	}
	network_printArray(arg, samples+start, size);
}

static void network_dipsFunc(struct network_CmdArg* arg)
{
	samplesize_t nDips = sampler_dips();
	snprintf(buffer, MSG_MAX_LEN-1, "# Dips = %d.\n", nDips);
	udp_send(socketDescriptor, buffer, arg->addr);
}

static void network_stopFunc(struct network_CmdArg* arg)
{
	main_exit(EXIT_SUCCESS);
}

static void network_helpFunc(struct network_CmdArg* arg)
{
	const int32_t displayNCmds = NETWORK_N_CMDS - 1;
	for (int32_t i = 0; i <  displayNCmds; i++) {
		snprintf(buffer, MSG_MAX_LEN-1, "%-10s -- %s\n", commands[i].name, commands[i].description);
		udp_send(socketDescriptor, buffer, arg->addr);
	}
	snprintf(buffer, MSG_MAX_LEN-1, "\n");
	udp_send(socketDescriptor, buffer, arg->addr);
}

static void network_defaultFunc(struct network_CmdArg* arg)
{
	udp_send(socketDescriptor, "Unknown command\n", arg->addr);
}

static void network_printArray(struct network_CmdArg* arg, lightsample_t* samples, const samplesize_t size)
{	
	const int32_t lineSize = 10;
	const int32_t lineMod = lineSize - 1;
	bool newLine = false;
	for (int32_t i = 0; i < size; i++) {
		newLine = (0 < i) && ((i % lineMod) == 0);
		if (newLine) {
			snprintf(buffer, MSG_MAX_LEN, "\n");
			udp_send(socketDescriptor, buffer, arg->addr);
		}
		snprintf(buffer, MSG_MAX_LEN, "%0.3f, ", samples[i]);
		udp_send(socketDescriptor, buffer, arg->addr);
	}
	snprintf(buffer, MSG_MAX_LEN, "\n\n");
	udp_send(socketDescriptor, buffer, arg->addr);
}