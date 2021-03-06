#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "a2d.h"

#define SOFT_ASSERT(b) {if(!(b)) {return A2D_ERR;}}

static const uint32_t A2D_MAX_PATHSIZE = 64;

static enum a2d_ErrCode a2d_readRawImpl(raw_t* output, const a2dfile_t fileNumber);
static volt_t a2d_rawToVoltImpl(raw_t rawVal);
static inline void a2d_generatePath(char* dest, const a2dfile_t fileNumber);

enum a2d_ErrCode a2d_readRaw(raw_t* output, const a2dfile_t fileNumber)
{
	return a2d_readRawImpl(output, fileNumber);
}

volt_t a2d_rawToVolt(raw_t rawVal)
{
	return a2d_rawToVoltImpl(rawVal);
}


static enum a2d_ErrCode a2d_readRawImpl(raw_t* output, const a2dfile_t fileNumber)
{
	SOFT_ASSERT(0 <= fileNumber && fileNumber < A2D_N_INFILES);
	char path[A2D_MAX_PATHSIZE];
	a2d_generatePath(path, fileNumber);
	FILE* f = fopen(path, "r");
	SOFT_ASSERT(f);
	int32_t itemsRead = fscanf(f, "%d", output);
	fclose(f);
	SOFT_ASSERT(itemsRead >= 0);
	return A2D_OK;
}

static volt_t a2d_rawToVoltImpl(raw_t rawVal)
{
	volt_t voltage = ((volt_t)rawVal / A2D_MAX_RAW) * A2D_MAX_VOLT;
	return voltage;
}

static inline void a2d_generatePath(char* dest, const a2dfile_t fileNumber)
{
	const char A2D_PATH[] = "/sys/bus/iio/devices/iio:device0";
	snprintf(dest, A2D_MAX_PATHSIZE, "%s/in_voltage%d_raw", A2D_PATH, fileNumber);
}