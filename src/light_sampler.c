#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "main.h"
#include "pot_driver.h"
#include "segdis_driver.h"
#include "light_sampler.h"

static struct CircleQueue queue;
static struct a2dping_Request pingpot;
static struct a2dping_Request pingsensor;

static void sampler_potFunc(const a2draw_t raw);
static void sampler_potErr(void);
static samplesize_t sampler_countDips(const lightsample_t* samples, const samplesize_t size, double avg);
static void* sampler_threadFunc(void* arg);
static void sampler_sampleFunc(const a2draw_t value);
static void* sampler_ledFuc(void* arg);

static pthread_t ledThread;
static pthread_t displayThread;
static a2draw_t potHist = 0;
static samplesize_t smplsPrS = 0;
static bool isRunning = false;
static double runningAvg = 0;

void sampler_init(void)
{
	const second_t potSIrvl = 1;
	const nanosecond_t potNSItrvl = 0;
	const second_t lightSIrvl = 0;
	const nanosecond_t lightNSItrvl = 1000000;
	circleQueue_init(&queue);
	pingpot = pot_newPingRequest(potSIrvl, potNSItrvl, sampler_potFunc, sampler_potErr);
	pingsensor = a2dping_newRequest(lightSIrvl, lightNSItrvl, 1, sampler_sampleFunc, NULL);
	a2dping_start(&pingpot);
	a2dping_start(&pingsensor);
	isRunning = true;
	pthread_create(&displayThread, NULL, sampler_threadFunc, NULL);
	pthread_create(&ledThread, NULL, sampler_ledFuc, NULL);
}

void sampler_cleanup(void)
{
	pthread_cancel(displayThread);
	pthread_cancel(ledThread);
	a2dping_stop(&pingpot);
	a2dping_stop(&pingsensor);
	circleQueue_free(&queue);
}

samplesize_t sampler_count(void)
{
	return queue.totalItems;
}

samplesize_t sampler_capacity(void)
{
	return circleQueue_capacity(&queue);
}

samplesize_t sampler_size(void)
{
	return circleQueue_size(&queue);
}

samplesize_t sampler_history(lightsample_t* dest, samplesize_t size)
{
	return circleQueue_dump(dest, size, &queue);
}

samplesize_t sampler_dips(void)
{
	lightsample_t samples[SAMPLER_MAX_HISTORY];
	samplesize_t sampleSize = circleQueue_dump(samples, SAMPLER_MAX_HISTORY, &queue);
	return sampler_countDips(samples, sampleSize, runningAvg);
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

static void sampler_potFunc(a2draw_t raw)
{
	a2draw_t diff = abs(raw - potHist);
	// prevent constant resizing due to noise
	if (diff > 1) {
		circleQueue_resize(&queue, raw);
		potHist = raw;
	}
}

static void sampler_potErr(void)
{
	const uint32_t errThreashold = 5;
	static uint32_t nErrs = 0;
	if (nErrs > errThreashold) {
		printf("Error reading POT\n");
		main_exit(EXIT_FAILURE);
	}
}

static samplesize_t sampler_countDips(const lightsample_t* samples, const samplesize_t size, const double avg)
{
	const lightsample_t threashold = 0.1;
	const lightsample_t hysteresis = threashold + 0.03;
	samplesize_t nDips = 0;
	double diff = 0;
	bool inDip = false;
	for (uint32_t i = 0; i < size; i++) {
		diff = avg - samples[i];
		if (threashold <= diff && !inDip) {
			inDip = true;
			nDips += 1;
		} else if (diff <= hysteresis && inDip) {
			inDip = false;
		}
	}
	return nDips;
}


static void sampler_sampleFunc(const a2draw_t value)
{

	const double weight = 0.01;
	const lightsample_t volt = a2d_rawToVolt(value);
	if (runningAvg == 0) {
		runningAvg = volt;
	} else {
		runningAvg = (weight * volt) + ((1.0-weight) * runningAvg);
	}
	smplsPrS += 1;
	circleQueue_insert(&queue, volt, true);
}

static void* sampler_threadFunc(void* arg)
{
	const second_t waitTime = 1;
	lightsample_t samples[SAMPLER_MAX_HISTORY];
	samplesize_t sampleSize = 0;
	samplesize_t nDips = 0;
	while (isRunning) {
		smplsPrS = 0;
		sleep(waitTime);
		sampleSize = circleQueue_dump(samples, SAMPLER_MAX_HISTORY, &queue);
		nDips = sampler_countDips(samples, sampleSize, runningAvg);
		printf("%d : %d : %d : %0.3f : %d\n", smplsPrS, potHist, sampleSize, runningAvg, nDips);
		for (int32_t i = 0; i < sampleSize; i++) {
			if (i > 0 && i % 200 == 0) {
				printf("%0.3f, ", samples[i]);
			}
		}
		printf("\n");
	}
	return NULL;
}

static void* sampler_ledFuc(void* arg)
{
	const second_t itvlS = 0;
	const nanosecond_t itvlNS = 100000000;
	lightsample_t samples[SAMPLER_MAX_HISTORY];
	samplesize_t sampleSize = 0;
	samplesize_t nDips = 0;
	struct timespec request = {itvlS, itvlNS};
	struct timespec remaining;
	while (isRunning) {
		sampleSize = circleQueue_dump(samples, SAMPLER_MAX_HISTORY, &queue);
		nDips = sampler_countDips(samples, sampleSize, runningAvg);
		segdis_setInt(nDips, false);
		nanosleep(&request, &remaining);
	}
	return NULL;
}