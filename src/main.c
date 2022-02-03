#include <stdio.h>
#include <unistd.h>

#include "pingpot.h"

void p1(int value) {
	printf("value: %d\n", value);
}

void p2(int value) {
	printf("peepee %d\n", value);
}

int main() {
	struct pingpot_Request r1 = pingpot_getRequest(0, 1, p1, NULL);
	struct pingpot_Request r2 = pingpot_getRequest(0, 1, p2, NULL);
	pingpot_request(&r1);
	sleep(2);
	pingpot_request(&r2);
	sleep(2);
	pingpot_kill(&r1);
	pingpot_kill(&r2);
}