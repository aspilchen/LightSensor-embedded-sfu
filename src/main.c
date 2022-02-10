#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "light_sampler.h"
#include "network.h"
#include "circleQueue.h"
#include "segdis_driver.h"

static int32_t exitCode = EXIT_SUCCESS;
static pthread_mutex_t mainlock = PTHREAD_MUTEX_INITIALIZER;

void main_exit(int32_t exitCode) {
	pthread_mutex_unlock(&mainlock);
}

int main() {
	segdis_init();
	sampler_init();
	network_init();
	pthread_mutex_lock(&mainlock);
	pthread_mutex_lock(&mainlock);
	network_cleanup();
	sampler_cleanup();
	segdis_cleanup();
	exit(exitCode);
}


