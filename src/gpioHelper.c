#include <stdint.h>
#include <stdio.h>

#include "gpioHelper.h"

#define GPIO_MAX_PATHSIZE 64

#define GPIO_PATH      "/sys/class/gpio/gpio"
#define GPIO_EXPORT    "/sys/class/gpio/export"
#define GPIO_VALUE     "value"
#define GPIO_DIRECTION "direction"

// If b is false, returns error code from containing function.
#define SOFT_ASSERT(b) {if(!(b)) {return GPIO_ERR;}}

static inline void gpio_generatePath(char* buffer, gpiopin_t pin, char* filename);
static enum gpio_ErrCode gpio_exportImpl(const gpiopin_t pin);
static enum gpio_ErrCode gpio_setDirImpl(const gpiopin_t pin, const char* direction);
static enum gpio_ErrCode gpio_readValueImpl(char* buffer, const gpiopin_t pin, const uint32_t size);
static enum gpio_ErrCode gpio_writeValueImpl(const gpiopin_t pin, char* buffer);


/*****************************************/
/********** interface functions **********/

enum gpio_ErrCode gpio_export(const gpiopin_t pin)
{
	return gpio_exportImpl(pin);
}

enum gpio_ErrCode gpio_setDir(const gpiopin_t pin, char* direction)
{
	return gpio_setDirImpl(pin, direction);
}

enum gpio_ErrCode gpio_readValue(char* buffer, const gpiopin_t pin, const uint32_t size)
{
	return gpio_readValueImpl(buffer, pin, size);
}

enum gpio_ErrCode gpio_writeValue(const gpiopin_t pin, char* buffer)
{
	return gpio_writeValueImpl(pin, buffer);
}

/**************************************/
/********** static functions **********/

static inline void gpio_generatePath(char* buffer, const gpiopin_t pin, char* filename)
{
	snprintf(buffer, GPIO_MAX_PATHSIZE, "%s%d/%s", GPIO_PATH, pin, filename);
}

static enum gpio_ErrCode gpio_exportImpl(const gpiopin_t pin)
{
	FILE *f = fopen("/sys/class/gpio/export", "w");
	SOFT_ASSERT(f);
	fprintf(f, "%d", pin);
	fclose(f);
	return GPIO_OK;
}

static enum gpio_ErrCode gpio_setDirImpl(const gpiopin_t pin, const char* direction)
{
	char path[GPIO_MAX_PATHSIZE];
	gpio_generatePath(path, pin, GPIO_DIRECTION);
	FILE *f = fopen(path, "w");
	SOFT_ASSERT(f);
	fprintf(f, "%s", direction);
	fclose(f);
	return GPIO_OK;
}

static enum gpio_ErrCode gpio_readValueImpl(char* buffer, gpiopin_t pin, uint32_t size)
{
	char path[GPIO_MAX_PATHSIZE];
	gpio_generatePath(path, pin, GPIO_VALUE);
	FILE *f = fopen(path, "r");
	SOFT_ASSERT(f);
	fgets(buffer, size, f);
	fclose(f);
	return GPIO_OK;
}

static enum gpio_ErrCode gpio_writeValueImpl(gpiopin_t pin, char* input)
{
	char path[GPIO_MAX_PATHSIZE];
	gpio_generatePath(path, pin, GPIO_VALUE);
	FILE *f = fopen(path, "w");
	SOFT_ASSERT(f);
	fprintf(f, "%s", input);
	fclose(f);
	return GPIO_OK;
}