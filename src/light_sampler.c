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

static struct CBuff historyBuffer;
static struct a2dping_Request pingpot;
static struct a2dping_Request pingsensor;

static void sampler_potFunc(const raw_t raw);
static void sampler_sampleFunc(const raw_t value);
static void* sampler_consoleDisplay(void* arg);
static void* sampler_ledDisplay(void* arg);

static void sampler_initImpl(void);
static void sampler_cleanupImpl(void);

static uint32_t sampler_countImpl(void);
static uint32_t sampler_capacityImpl(void);
static uint32_t sampler_sizeImpl(void);
static uint32_t sampler_historyImpl(volt_t* dest, uint32_t size);
static uint32_t sampler_dipsImpl(void);
static uint32_t sampler_countDips(const volt_t* samples, const uint32_t size);

static uint32_t TOTAL_SAMPLES = 0;
static uint32_t SAMPLES_PR_SECOND = 0;
static volt_t RUNNING_AVG = 0;
static raw_t POT_HIST = 0;

const a2dfile_t SENSOR_FILENO = 1;

static pthread_t ledThread;
static pthread_t displayThread;
static bool isRunning = false;

void sampler_init(void)
{
	sampler_initImpl();
}

void sampler_cleanup(void)
{
	sampler_cleanupImpl();
}

uint32_t sampler_count(void)
{
	return sampler_countImpl();
}

uint32_t sampler_capacity(void)
{
	return sampler_capacityImpl();
}

uint32_t sampler_size(void)
{
	return sampler_sizeImpl();
}

uint32_t sampler_history(volt_t* dest, uint32_t size)
{
	return sampler_historyImpl(dest, size);
}

uint32_t sampler_dips(void)
{
	return sampler_dipsImpl();
}

static void sampler_initImpl(void)
{
	static const second_t POT_INTERVAL_S = 1;
	static const nanosecond_t POT_INTERVAL_NS = 0;
	static const second_t LIGHT_INTERVAL_S = 0;
	static const nanosecond_t LIGHT_INTERVAL_NS = 1000000;
	POT_HIST = 0;
	RUNNING_AVG = 0;
	SAMPLES_PR_SECOND = 0;
	TOTAL_SAMPLES = 0;
	pingpot = pot_newPingRequest(POT_INTERVAL_S, POT_INTERVAL_NS, sampler_potFunc, NULL);
	pingsensor = a2dping_newRequest(LIGHT_INTERVAL_S, LIGHT_INTERVAL_NS, SENSOR_FILENO, sampler_sampleFunc, NULL);
	isRunning = true;
	cbuff_init(&historyBuffer);
	a2dping_start(&pingpot);
	a2dping_start(&pingsensor);
	pthread_create(&displayThread, NULL, sampler_consoleDisplay, NULL);
	pthread_create(&ledThread, NULL, sampler_ledDisplay, NULL);
}

static void sampler_cleanupImpl(void)
{
	pthread_cancel(displayThread);
	pthread_cancel(ledThread);
	a2dping_stop(&pingpot);
	a2dping_stop(&pingsensor);
	cbuff_destroy(&historyBuffer);
}

static uint32_t sampler_countImpl(void)
{
	return TOTAL_SAMPLES;
}

static uint32_t sampler_capacityImpl(void)
{
	return cbuff_capacity(&historyBuffer);
}

static uint32_t sampler_sizeImpl(void)
{
	return cbuff_size(&historyBuffer);
}

static uint32_t sampler_historyImpl(volt_t* dest, uint32_t size)
{
	return cbuff_dump(dest, &historyBuffer, size);
}

static uint32_t sampler_dipsImpl(void)
{
	volt_t samples[SAMPLER_MAX_HISTORY];
	uint32_t sampleSize = cbuff_dump(samples, &historyBuffer, SAMPLER_MAX_HISTORY);
	return sampler_countDips(samples, sampleSize);
}

static uint32_t sampler_countDips(const volt_t* samples, const uint32_t size)
{
	static const volt_t threashold = 0.1;
	static const volt_t hysteresis = threashold + 0.03;
	const volt_t average = RUNNING_AVG;
	volt_t diff = 0;
	uint32_t nDips = 0;
	bool inDip = false;
	for (index_t i = 0; i < size; i++) {
		diff = average - samples[i];
		if (threashold <= diff && !inDip) {
			inDip = true;
			nDips += 1;
		} else if (diff <= hysteresis && inDip) {
			inDip = false;
		}
	}
	return nDips;
}

static void sampler_potFunc(raw_t raw)
{
	raw_t diff = abs(raw - POT_HIST);
	// prevent constant resizing due to noise
	if (diff > 1) {
		cbuff_resize(&historyBuffer, raw);
		POT_HIST = raw;
	}
}

static void* sampler_ledDisplay(void* arg)
{
	const second_t LED_REFRESH_S = 0;
	const nanosecond_t LED_REFRESH_NS = 100000000;
	volt_t samples[SAMPLER_MAX_HISTORY];
	uint32_t sampleSize = 0;
	uint32_t nDips = 0;
	struct timespec request = {LED_REFRESH_S, LED_REFRESH_NS};
	struct timespec remaining;
	while (isRunning) {
		sampleSize = cbuff_dump(samples, &historyBuffer, SAMPLER_MAX_HISTORY);
		nDips = sampler_countDips(samples, sampleSize);
		segdis_setInt(nDips, false);
		nanosleep(&request, &remaining);
	}
	return NULL;
}

static void* sampler_consoleDisplay(void* arg)
{
	static const second_t INTERVAL_SECONDS = 1;
	volt_t samples[SAMPLER_MAX_HISTORY];
	uint32_t sampleSize = 0;
	uint32_t nDips = 0;
	while (isRunning) {
		SAMPLES_PR_SECOND = 0;
		sampleSize = cbuff_dump(samples, &historyBuffer, SAMPLER_MAX_HISTORY);
		sleep(INTERVAL_SECONDS);
		nDips = sampler_countDips(samples, sampleSize);
		printf("%d : %d : %d : %0.3f : %d\n"
			  ,SAMPLES_PR_SECOND
			  ,POT_HIST
			  ,sampleSize
			  ,RUNNING_AVG
			  ,nDips);
		for (int32_t i = 0; i < sampleSize; i++) {
			if (i > 0 && i % 200 == 0) {
				printf("%0.3f, ", samples[i]);
			}
		}
		printf("\n");
	}
	return NULL;
}

static void sampler_sampleFunc(const raw_t value)
{
	const double weight = 0.01;
	const volt_t volt = a2d_rawToVolt(value);
	if (RUNNING_AVG == 0) {
		RUNNING_AVG = volt;
	} else {
		RUNNING_AVG = (weight * volt) + ((1.0-weight) * RUNNING_AVG);
	}
	SAMPLES_PR_SECOND += 1;
	TOTAL_SAMPLES += 1;
	cbuff_insert(&historyBuffer, volt);
}