#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "pingpot.h"
#include "light_sampler.h"

static struct CircleQueue queue;
static pthread_t samplerThread;
static bool isRunning = true;

static void* sampler_threadFunc(void* arg);
static lightsample_t sampler_readLightLevel(void);

void sampler_init(void)
{
	const samplesize_t initQueueSize = 1000;
	circleQueue_init(&queue, initQueueSize);
	pthread_create(&samplerThread, NULL, sampler_threadFunc, NULL);
}

void sampler_cleanup(void)
{
	isRunning = false;
	pthread_join(samplerThread, NULL);
	circleQueue_free(&queue);
}

uint64_t sampler_count(void)
{
	return queue.totalItems;
}

void sampler_length(samplesize_t* capacity, samplesize_t* size)
{
	*capacity = circleQueue_capacity(&queue);
	*size = circleQueue_size(&queue);
}

void sampler_history(lightsample_t* dest, samplesize_t size)
{
	samplesize_t queueSize = circleQueue_size(&queue);
	sampler_get(dest, size, queueSize);
}

void sampler_get(lightsample_t* dest, const samplesize_t size, const samplesize_t n)
{
	circleQueue_lock(&queue);
	samplesize_t i = circleQueue_size(&queue) - n;
	queueindex_t mappedIdx = circleQueue_map(&queue, i);
	if (i < 0) {
		return;
	}
	while (0 <= mappedIdx && i < size) {
		dest[i] = queue.data[mappedIdx];
		i += 1;
		mappedIdx = circleQueue_map(&queue, i);
	}
	circleQueue_unlock(&queue);
}

uint32_t sampler_dips(void)
{

} 
void sampler_cleanup(void);

static void sampler_potFunc(a2draw_t value)
{
	static a2draw_t valHist;
	a2draw_t diff = abs(value - valHist);
	// prevent constant resizing due to noise
	if (diff > 1) {
		circleQueue_resize(&queue, value);
	}
	valHist = value;
}

static void* sampler_threadFunc(void* arg)
{
	const second_t potInterval = 1;
	struct pingpot_Request potRequest = pingpot_getRequest(potInterval, 0, sampler_potFunc, NULL);
	lightsample_t sample;
	while (isRunning) {
		sample = sampler_readLightLevel();
		circleQueue_insert(&queue, sample);
	}
	pingpot_kill(&potRequest);
}

static lightsample_t sampler_readLightLevel(void)
{
	return 22;
}

