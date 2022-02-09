// Modified version of demo driver provided on class page.
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "potDriver.h"

// If b is false, returns error code from containing function.
#define SOFT_ASSERT(b) {if(!(b)) {return POT_ERR;}}

static const char POT_A2D_FILE[]	= "/sys/bus/iio/devices/iio:device0/in_voltage0_raw";

static enum pot_ErrCode potDriver_readRawImpl(int32_t* output);
static enum pot_ErrCode potDriver_readVoltImpl(double* output);

/*****************************************/
/********** interface functions **********/

enum pot_ErrCode potDriver_readRaw(int32_t* output)
{
	return potDriver_readRawImpl(output);
}

enum pot_ErrCode potDriver_readVolt(double* output)
{
	return potDriver_readVoltImpl(output);
}

/**************************************/
/********** static functions **********/

static enum pot_ErrCode potDriver_readRawImpl(int32_t* output)
{
	FILE* f = fopen(POT_A2D_FILE, "r");
	SOFT_ASSERT(f);
	int32_t itemsRead = fscanf(f, "%d", output);
	fclose(f);
	SOFT_ASSERT(itemsRead >= 0);
	return POT_OK;
}

static enum pot_ErrCode potDriver_readVoltImpl(double* output)
{
	int32_t rawValue = 0;
	enum pot_ErrCode errCode = potDriver_readRawImpl(&rawValue);
	*output = ((double)rawValue / POT_MAX_VAL) * POT_MAX_VOLT;
	return errCode;
}