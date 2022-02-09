#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <math.h>

#include "i2cHelper.h"
#include "gpioHelper.h"
#include "segdisDriver.h"


/*******************************/
/********** variables **********/

// Tried to do this with const variables but the compiler gets angry about discarding const.
#define MAX_POSITIONS 2
#define LEFT_IDX 0
#define TOP_IDX  0
#define RGHT_IDX 1
#define BTM_IDX  1
#define WAIT_TIME_MS 5000000
#define SEGDIS_ON "1"
#define SEGDIS_OFF "0"
#define I2C_DEVICE 0x20

enum PosReg {
	REG_TOP = 0x15,
	REG_BOTTOM = 0x14,
	N_REGS = 2
};

enum Pin {
	PIN_LEFT = 61,
	PIN_RIGHT = 44,
	N_PINS = 2
};

static const enum Pin GPIO_PINS[] = {PIN_LEFT, PIN_RIGHT};
static const enum PosReg OUT_REGS[] = {REG_TOP, REG_BOTTOM};

#define N_INTS 10
static const unsigned char PRESET_INTS[][MAX_POSITIONS] = {
// {top, bottom}
	{0x86, 0xA1}, // 0
	{0x12, 0x80}, // 1
	{0x0E, 0x31}, // 2
	{0x06, 0xB0}, // 3
	{0x8A, 0x90}, // 4
	{0x8C, 0xB0}, // 5
	{0x8C, 0xB1}, // 6
	{0x14, 0x04}, // 7
	{0x8E, 0xB1}, // 8
	{0x8E, 0x90}, // 9
};

static pthread_t displayThread;
static bool isRunning = true; //              left (top , bot)   (top , bot) right
static unsigned char valueCache[][MAX_POSITIONS] = {{0x00, 0x00}, {0x00, 0x00}};
static int i2cBusFD = 0;

/***************************************/
/********** interface headers **********/

static void segdis_i2cInit(void);
static void segdis_gpioInit(void);
static void* segdis_display(void* arg);
static void segdis_writeFromCach(const unsigned int pinIdx);
static void segdis_cachInt(const unsigned int n, const bool withDecimal);
static void segdis_i2cCleanup(void);
static void segdis_gpioCleanup(void);
static inline void segdis_turnOn(const enum Pin pin);
static inline void segdis_turnOff(const enum Pin pin);

/*****************************************/
/********** interface functions **********/

void segdis_init(void)
{
	segdis_i2cInit();
	segdis_gpioInit();
	gpio_writeValue(PIN_LEFT, SEGDIS_ON);
	pthread_create(&displayThread, NULL, segdis_display, NULL);
}

void segdis_setInt(const unsigned int n, const bool withDecimal)
{
	segdis_cachInt(n, withDecimal);
}

void segdis_cleanup(void)
{
	isRunning = false;
	pthread_join(displayThread, NULL);
	segdis_gpioCleanup();
	segdis_i2cCleanup();
}

/**************************************/
/********** static functions **********/

static void segdis_i2cInit(void)
{
	static const int32_t nSysCommands = 2;
	static const char* sysCommands[] = {"config-pin P9_18 i2c", "config-pin P9_17 i2c"};
	static const int dirRegs[] = {0x00, 0x01};
	const char* bus = I2C_BUS1;
	for (int32_t i = 0; i < nSysCommands; i++) {
		system(sysCommands[i]);
	}
	i2ch_initBus(&i2cBusFD, bus, I2C_DEVICE);
	for (int i = 0; i < MAX_POSITIONS; i++) {
		i2ch_writeReg(i2cBusFD, OUT_REGS[i], 0x00);
		i2ch_writeReg(i2cBusFD, dirRegs[i], 0x00);
	}
}

static void segdis_gpioInit(void)
{
	for (int32_t i = 0; i < N_PINS; i++) {
		gpio_export(GPIO_PINS[i]);
		gpio_setDir(GPIO_PINS[i], GPIO_OUT);
		segdis_turnOn(GPIO_PINS[i]);
	}
}

static void* segdis_display(void* arg)
{
	struct timespec request = {0, WAIT_TIME_MS};
	struct timespec remaining;
	while (isRunning) {
		segdis_turnOff(PIN_RIGHT);
		segdis_writeFromCach(LEFT_IDX);
		segdis_turnOn(PIN_LEFT);
		nanosleep(&request, &remaining);
		segdis_turnOff(PIN_LEFT);
		segdis_writeFromCach(RGHT_IDX);
		segdis_turnOn(PIN_RIGHT);
		nanosleep(&request, &remaining);
	}
	return NULL;
}

static inline void segdis_turnOn(const enum Pin pin)
{
	gpio_writeValue(pin, SEGDIS_ON);
}

static inline void segdis_turnOff(const enum Pin pin)
{
	gpio_writeValue(pin, SEGDIS_OFF);
}

static void segdis_writeFromCach(const unsigned int pinIdx)
{
	i2ch_writeReg(i2cBusFD, REG_TOP, valueCache[pinIdx][TOP_IDX]);
	i2ch_writeReg(i2cBusFD, REG_BOTTOM, valueCache[pinIdx][BTM_IDX]);
}

static void segdis_cachInt(const unsigned int n, const bool withDecimal)
{
	const unsigned char decimal = 0x40;
	const unsigned int rightDigit = n % 10;
	const unsigned int leftDigit = (n - rightDigit) / 10;
	const int size = sizeof(PRESET_INTS[0][0]) * MAX_POSITIONS;
	memcpy(valueCache[0], PRESET_INTS[leftDigit], size);
	memcpy(valueCache[1], PRESET_INTS[rightDigit], size);
	if (withDecimal) {
		valueCache[LEFT_IDX][BTM_IDX] = valueCache[LEFT_IDX][BTM_IDX] | decimal;
	}
}


static void segdis_i2cCleanup(void)
{
	close(i2cBusFD);
}

static void segdis_gpioCleanup(void)
{
	for (int i = 0; i < MAX_POSITIONS; i++) {
		segdis_turnOff(GPIO_PINS[i]);
	}
}