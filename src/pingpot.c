#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include "pingpot.h"

static struct pingpot_Request pingpot_getRequestImpl(int64_t nPings, double interval, void (*rawCB) (int32_t output), void (*voltageCB) (double output));
static void pingpot_requestImpl(struct pingpot_Request* request);
static void pingpot_killImpl(const struct pingpot_Request* request);
static void* unlimited_thread(void* arg);
// static void limited_thread(const struct pingpot_Request* request);
static bool isValidRequest(const struct pingpot_Request* request);

static bool isActiveThread[PINGPOT_MAX_REQUESTS] = {false, false, false, false, false};
static pthread_t threads[PINGPOT_MAX_REQUESTS];


struct pingpot_Request pingpot_getRequest(int64_t nPings, double interval, void (*rawCB) (int32_t output), void (*voltageCB) (double output))
{
	return pingpot_getRequestImpl(nPings, interval, rawCB, voltageCB);
}

void pingpot_request(struct pingpot_Request* request)
{
	pingpot_requestImpl(request);
}

static struct pingpot_Request pingpot_getRequestImpl(int64_t nPings, double interval, void (*rawCB) (int32_t output), void (*voltageCB) (double output))
{
	int32_t id = -1;
	for (int32_t i = 0; i < PINGPOT_MAX_REQUESTS; i++) {
		if (!isActiveThread[i]) {
			id = i;
			break;
		}
	}
	struct pingpot_Request request = {
		nPings,
		id,
		interval,
		rawCB,
		voltageCB
	};
	return request;
}

void pingpot_kill(const struct pingpot_Request* request)
{
	pingpot_killImpl(request);
}

static void pingpot_requestImpl(struct pingpot_Request* request)
{
	bool isUnlimited = 0 <= request->nPings;
	if (isValidRequest(request)) {
		if (isUnlimited) {
			isActiveThread[request->id] = true;
			pthread_create(&threads[request->id], NULL, unlimited_thread, request);
		}
	}
}

static void* unlimited_thread(void* arg)
{
	struct pingpot_Request* request = (struct pingpot_Request*)arg;
	int rawReading = 0;
	double voltageReading = 0;
	while (isActiveThread[request->id]) {
		if (request->intCB) {
			rawReading = potDriver_readRaw();
			request->intCB(rawReading);
		}
		if (request->doubleCB) {
			voltageReading = potDriver_readVolt();
			request->doubleCB(voltageReading);
		}
		sleep(request->interval);
	}
	return NULL;
}

static void pingpot_killImpl(const struct pingpot_Request* request)
{
	if (isValidRequest(request) && isActiveThread[request->id]) {
		isActiveThread[request->id] = false;
		pthread_join(threads[request->id], NULL);
	}
}

static bool isValidRequest(const struct pingpot_Request* request)
{
	return 0 <= request->id && request->id < PINGPOT_MAX_REQUESTS;
}