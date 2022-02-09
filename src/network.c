/*
 * UDP Listening program on port 22110
 * By Brian Fraser, Modified from Linux Programming Unleashed (book)
 *
 * Usage:
 *	On the target, run this program (netListenTest).
 *	On the host:
 *		> netcat -u 192.168.0.171 22110
 *		(Change the IP address to your board)
 *
 *	On the host, type in a number and press enter:
 *		4<ENTER>
 *
 *	On the target, you'll see a debug message:
 *	    Message received (2 bytes):
 *	    '4
 *	    '
 *
 *	On the host, you'll see the message:
 *	    Math: 4 + 1 = 5
 *
 */

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

/*
 * Accepted commands
 * help
 * Return a brief summary/list of supported commands.
 * count
 * Return the total number of light samples take so far (may be huge, like > 10 billion).
 * get N
 * Return the N most recent sample history values.
 * Display an error if N > number of samples currently in the history; indicate the valid
 * range of values.
 * length
 * Return both the max size of the history, and the number of samples currently in the
 * history. These values will not be the same if the buffer is not yet full.
 * history
 * Return all the data samples in the history.
 * Values must be the voltage of the sample, displayed to 3 decimal places.
 * Values must be comma separated, and display 20 numbers per line.
 * Send multiple return packets if the history is too big for one packet.
 * You can assume that 1,500 bytes of data will fit into a UDP packet. This works
 * across Ethernet over USB.
 * No single sample may have its digits split across two packets.
 * stop
 * Exit the program.
 * Must shutdown gracefully: close all open sockets, files, pipes, threads, and free all
 * dynamically allocated memory.
 * <enter>
 * A blank input (which will actually be a line-feed) should repeat the previous
 * command. If sent as the first command, treat as an unknown command.
 * All unknown commands return a message indicating it's unknown.
*/

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
	char buffer[MSG_MAX_LEN];
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

static int32_t network_parseCommand(char* buffer)
{
	int32_t cmdNumber = 0;
	buffer = strtok(buffer, " ");
	for (cmdNumber = 0; cmdNumber < NETWORK_N_CMDS; cmdNumber++) {
		if (strcmp(commands[cmdNumber].symbol, buffer) == 0) {
			break;
		}
	}
	buffer = strtok(NULL, " ");
	return cmdNumber;
}

static inline void network_act(int32_t cmdNum, struct network_CmdArg* arg)
{
	commands[NETWORK_REPEAT_CMD].cmdAction = commands[cmdNum].cmdAction;
	commands[cmdNum].cmdAction(arg);
}

static void network_countFunc(struct network_CmdArg* arg)
{
	char buffer[MSG_MAX_LEN];
	int32_t count = sampler_count();
	snprintf(buffer, MSG_MAX_LEN-1, "Number of samples taken = %d.\n", count);
	udp_send(socketDescriptor, buffer, arg->addr);
}

static void network_lengthFunc(struct network_CmdArg* arg)
{
	
}

static void network_historyFunc(struct network_CmdArg* arg)
{
	
}

static void network_getFunc(struct network_CmdArg* arg)
{
	
}

static void network_dipsFunc(struct network_CmdArg* arg)
{
	
}

static void network_stopFunc(struct network_CmdArg* arg)
{
	main_exit(EXIT_SUCCESS);
}

static void network_helpFunc(struct network_CmdArg* arg)
{
	const int32_t displayNCmds = NETWORK_N_CMDS - 1;
	char buffer[MSG_MAX_LEN];
	for (int32_t i = 0; i <  displayNCmds; i++) {
		snprintf(buffer, MSG_MAX_LEN-1, "%-10s %s %s\n", commands[i].name, "--", commands[i].description);
		udp_send(socketDescriptor, buffer, arg->addr);
	}
}

static void network_defaultFunc(struct network_CmdArg* arg)
{
	udp_send(socketDescriptor, "Unknown command\n", arg->addr);
}