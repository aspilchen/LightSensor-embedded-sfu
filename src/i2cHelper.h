#ifndef I2CH_H
#define I2CH_H

#include <stdint.h>

#define I2C_BUS0 "/dev/i2c-0"
#define I2C_BUS1 "/dev/i2c-1"
#define I2C_BUS2 "/dev/i2c-2"

typedef uint8_t i2cval_t;
typedef int32_t i2cfile_t;

enum i2ch_ErrCode {
	I2C_OK,
	I2C_ERR
};

enum i2ch_ErrCode i2ch_initBus(i2cfile_t* outFD, const char* bus, const i2cval_t address);
enum i2ch_ErrCode i2ch_writeReg(const i2cfile_t i2cFD, const i2cval_t regAddr, const i2cval_t value);
enum i2ch_ErrCode i2ch_readReg(char* output, const i2cfile_t i2cFileDesc, const i2cval_t regAddr);

#endif