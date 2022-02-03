#ifndef POT_H
#define POT_H

#define PINGPOT_MAX_REQUESTS 5

#include "potDriver.h"

struct pingpot_Request {
	int32_t id;
	int64_t nPings;
	double interval;
	void (*intCB) (int output);
	void (*doubleCB) (double output);
};

struct pingpot_Request pingpot_getRequest(int64_t nPings, double interval, void (*rawCB) (int32_t output), void (*voltageCB) (double output));
void pingpot_request(struct pingpot_Request* request);
void pingpot_kill(const struct pingpot_Request* request);

#endif