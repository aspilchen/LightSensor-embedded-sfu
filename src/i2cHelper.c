#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "i2cHelper.h"

// If b is false, returns error code from containing function.
#define SOFT_ASSERT(b) {if(!(b)) {return I2C_ERR;}}

static enum i2ch_ErrCode i2ch_initBusImpl(i2cfile_t* outFD, const char* bus, const i2cval_t address);
static enum i2ch_ErrCode i2ch_writeRegImpl(const i2cfile_t i2cFD, const i2cval_t regAddr, const i2cval_t value);
static enum i2ch_ErrCode i2ch_readRegImpl(char* output, const i2cfile_t i2cFileDesc, const i2cval_t regAddr);

/*****************************************/
/********** interface functions **********/

enum i2ch_ErrCode i2ch_initBus(i2cfile_t* outFD, const char* bus, const i2cval_t address)
{
	return i2ch_initBusImpl(outFD, bus, address);
}

enum i2ch_ErrCode i2ch_writeReg(const i2cfile_t i2cFD, const i2cval_t regAddr, const i2cval_t value)
{
	return i2ch_writeRegImpl(i2cFD, regAddr, value);
}

enum i2ch_ErrCode i2ch_readReg(char* output, const i2cfile_t i2cFD, const i2cval_t regAddr)
{
	return i2ch_readRegImpl(output, i2cFD, regAddr);
}

/**************************************/
/********** static functions **********/

static enum i2ch_ErrCode i2ch_initBusImpl(i2cfile_t* outFD, const char* bus, const i2cval_t address)
{
	*outFD = open(bus, O_RDWR);
	SOFT_ASSERT(*outFD);
	int32_t result = ioctl(*outFD, I2C_SLAVE, address);
	SOFT_ASSERT(result >= 0);
	return I2C_OK;
}

static enum i2ch_ErrCode i2ch_writeRegImpl(const i2cfile_t i2cFD, const i2cval_t regAddr, const i2cval_t value)
{
	i2cval_t buffer[2];
	buffer[0] = regAddr;
	buffer[1] = value;
	int32_t result = write(i2cFD, buffer, 2);
	SOFT_ASSERT(result == 2);
	return I2C_OK;
}

static enum i2ch_ErrCode i2ch_readRegImpl(char* output, const i2cfile_t i2cFD, const i2cval_t regAddr)
{
	int32_t result = write(i2cFD, &regAddr, sizeof(regAddr));
	SOFT_ASSERT(result == sizeof(regAddr));
	result = read(i2cFD, output, sizeof(*output));
	SOFT_ASSERT(result == sizeof(*output));
	return I2C_OK;
}
