// Modified version of demo driver provided on class page.
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "potDriver.h"

static enum pot_ErrCode ERR_CODE = POT_OK;

static const double POT_A2D_VOLTAGE = 1.8;
static const char POT_A2D_FILE[]	= "/sys/bus/iio/devices/iio:device0/in_voltage0_raw";
static const int POT_A2D_MAX	    = 4095;


static inline FILE* potDriver_fopen();
static inline int potDriver_read(FILE* f);

int32_t potDriver_readRaw()
{
	int32_t value = 0;
	FILE* f = potDriver_fopen();
	if (!f) {
		return POT_ERROR;
	}
	value = potDriver_read(f);
	fclose(f);
	if (value < 0) {
		return POT_ERROR;
	}
	return value;
}

int64_t potDriver_readVolt()
{
	int64_t voltage = 0;
	int32_t rawValue = 0;
	FILE* f = potDriver_fopen();
	if (!f) {
		return POT_ERROR;
	}
	rawValue = potDriver_read(f);
	fclose(f);
	if (rawValue < 0) {
		return POT_ERROR;
	}
	voltage = ((double)rawValue / POT_A2D_MAX) * POT_A2D_VOLTAGE;
	return voltage;
}

static inline FILE* potDriver_fopen()
{
	FILE *f = fopen(POT_A2D_FILE, "r");
	if (!f) {
		ERR_CODE = POT_FOPEN_ERR;
	}
	return f;
}

static inline int potDriver_read(FILE* f)
{
	int32_t value;
	int32_t itemsRead = fscanf(f, "%d", &value);
	if (itemsRead <= 0) {
		ERR_CODE = POT_READ_ERR;
		return POT_ERROR;
	}
	return value;
}