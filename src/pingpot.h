#ifndef PINGPOT_H
#define PINGPOT_H

#include <stdint.h>
#include <stdbool.h>

typedef int32_t a2draw_t;
typedef double a2dvoltage_t;
typedef int32_t second_t;
typedef int64_t nanosecond_t;
// Array size is defined through this. Had to be a macro
#define POT_MAX_VAL 4095

struct pingpot_Request {
	second_t timeS;
	nanosecond_t timeMs;
	void (*intFunc) (const a2draw_t output);
	void (*doubleFunc) (const a2dvoltage_t output);
	bool isActive;
	pthread_t thread;
};

struct pingpot_Request pingpot_getRequest(const second_t timeS
										 ,const int64_t timeNS
										 ,void (*rawFunc)(const a2draw_t output)
										 ,void (*voltageFunc)(const a2dvoltage_t output));

void pingpot_ping(struct pingpot_Request* request);
void pingpot_kill(struct pingpot_Request* request);

#endif