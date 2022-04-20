#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "a2dping.h"

static struct a2dping_Request a2dping_newRequestImpl(const second_t timeS
													,const nanosecond_t timeNS
													,const a2dfile_t fileno
													,void (*callback)(const raw_t output)
													,void (*handleErr)(void));

static void  a2dping_startImpl(struct a2dping_Request* request);
static void  a2dping_stopImpl(struct a2dping_Request* request);
static void* a2dping_threadFunc(void* arg);


/*****************************************/
/********** interface functions **********/
struct a2dping_Request a2dping_newRequest(const second_t timeS
										 ,const nanosecond_t timeNS
										 ,const a2dfile_t fileno
										 ,void (*callback)(const raw_t output)
										 ,void (*handleErr)(void))
{
	return a2dping_newRequestImpl(timeS, timeNS, fileno, callback, handleErr);
}

void a2dping_start(struct a2dping_Request* request)
{
	a2dping_startImpl(request);
}

void a2dping_stop(struct a2dping_Request* request)
{
	a2dping_stopImpl(request);
}

/**************************************/
/********** static functions **********/

static struct a2dping_Request a2dping_newRequestImpl(const second_t timeS
													,const nanosecond_t timeNS
													,const a2dfile_t fileno
													,void (*callback)(const raw_t value)
													,void (*handleErr)(void))
{
	struct a2dping_Request request = {
		timeS
		,timeNS
		,fileno
		,callback
		,handleErr
		,false
	};
	return request;
}

static void a2dping_startImpl(struct a2dping_Request* request)
{
	if (!request->isActive) {
		request->isActive = true;
		pthread_create(&(request->thread), NULL, a2dping_threadFunc, request);
	}
}

static void* a2dping_threadFunc(void* arg)
{
	struct a2dping_Request* request = (struct a2dping_Request*)arg;
	enum a2d_ErrCode errcode;
	raw_t raw = 0;
	struct timespec req = {request->timeS, request->timeMs};
	struct timespec remaining; 
	while (request->isActive) {
		errcode = a2d_readRaw(&raw, request->fileno);
		if (errcode == A2D_OK) {
			request->callback(raw);
		} else if (request->handleErr) {
			request->handleErr();
		}
		nanosleep(&req, &remaining);
	}
	return NULL;
}

static void a2dping_stopImpl(struct a2dping_Request* request)
{
	request->isActive = false;
	pthread_join(request->thread, NULL);
}