#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "pingpot.h"
// #include "segdisDriver.h"
// #include "light_sampler.h"
// #include "network.h"

static pthread_mutex_t mainMutex = PTHREAD_MUTEX_INITIALIZER;
static int exitcode = EXIT_SUCCESS;
static bool isRunning = true;

void potfunc(const int32_t value) {
	printf("raw %d\n", value);
}

void pf(const double value) {
	printf("voltage %f\n", value);
}

void main_exit(const int exitCode)
{
	exitcode = exitCode;
	pthread_mutex_unlock(&mainMutex);
}

int main() {
	struct pingpot_Request req = pingpot_getRequest(1, 0, potfunc, pf);
	pingpot_ping(&req);
	sleep(15);
	pingpot_kill(&req);
	//segdis_init();
	//network_init();
	//sampler_init();
	//pthread_mutex_lock(&mainMutex);
	//pthread_mutex_lock(&mainMutex);
	//segdis_cleanup();
	//network_cleanup();
	//exit(exitcode);
}

