#ifndef A2DREQ_H
#define A2DREQ_H
/**
 * a2dping uses a2dping_Request objects to hold data about a specific ping thread.
 * Included in any drivers I make. This is the raw interface, drivers will generate
 * requests to their own files.
 */

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#include "a2d.h"

typedef int32_t second_t;
typedef int64_t nanosecond_t;

struct a2dping_Request {
	second_t timeS;
	nanosecond_t timeMs;
	a2dfile_t fileno;
	void (*callback) (const a2draw_t output);
	void (*handleErr) (void);
	bool isActive;
	pthread_t thread;
};

struct a2dping_Request a2dping_newRequest(const second_t timeS
										 ,const int64_t timeNS
										 ,const a2dfile_t fileno
										 ,void (*callback)(const a2draw_t output)
										 ,void (*handleErr)(void));

void a2dping_start(struct a2dping_Request* request);
void a2dping_stop(struct a2dping_Request* request);

#endif