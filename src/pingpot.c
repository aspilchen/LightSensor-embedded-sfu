#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "potDriver.h"
#include "pingpot.h"

struct pingpot_Request pingpot_getRequestImpl(const second_t timeS
											 ,const nanosecond_t timeNS
											 ,void (*rawFunc)(const a2draw_t output)
											 ,void (*voltageFunc)(const a2dvoltage_t output));

static void pingpot_pingImpl(struct pingpot_Request* request);
static void pingpot_killImpl(struct pingpot_Request* request);
static void* pingpot_threadFunc(void* arg);


/*****************************************/
/********** interface functions **********/
struct pingpot_Request pingpot_getRequest(const second_t timeS
										 ,const nanosecond_t timeNS
										 ,void (*rawFunc)(const a2draw_t output)
										 ,void (*voltageFunc)(const a2dvoltage_t output))
{
	return pingpot_getRequestImpl(timeS, timeNS, rawFunc, voltageFunc);
}

void pingpot_ping(struct pingpot_Request* request)
{
	pingpot_pingImpl(request);
}

/**************************************/
/********** static functions **********/

struct pingpot_Request pingpot_getRequestImpl(const second_t timeS
											 ,const nanosecond_t timeNS
											 ,void (*rawFunc)(const a2draw_t value)
											 ,void (*voltageFunc)(const a2dvoltage_t value))
{
	struct pingpot_Request request = {
		timeS,
		timeNS,
		rawFunc,
		voltageFunc,
		false
	};
	return request;
}

void pingpot_kill(struct pingpot_Request* request)
{
	pingpot_killImpl(request);
}

static void pingpot_pingImpl(struct pingpot_Request* request)
{
	if (!request->isActive) {
		pthread_create(&(request->thread), NULL, pingpot_threadFunc, request);
	}
}

static void* pingpot_threadFunc(void* arg)
{
	struct pingpot_Request* request = (struct pingpot_Request*)arg;
	a2draw_t rawReading = 0;
	a2dvoltage_t voltageReading = 0.0;
	struct timespec req = {request->timeS, request->timeMs};
	struct timespec remaining; 
	request->isActive = true;
	while (request->isActive) {
		potDriver_readVolt(&voltageReading);
		if (request->intFunc) {
			potDriver_readRaw(&rawReading);
			request->intFunc(rawReading);
		}
		if (request->doubleFunc) {
			request->doubleFunc(voltageReading);
		}
		nanosleep(&req, &remaining);
	}
	return NULL;
}

static void pingpot_killImpl(struct pingpot_Request* request)
{
	request->isActive = false;
	pthread_join(request->thread, NULL);
}