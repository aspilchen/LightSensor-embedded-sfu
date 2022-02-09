#ifndef POTDRIVER_H
#define POTDRIVER_H

#include <stdint.h>

#define POT_MAX_VAL 4095
#define POT_MAX_VOLT 1.8

enum pot_ErrCode {
	POT_OK,
	POT_ERR
};

enum pot_ErrCode potDriver_readRaw(int32_t* output);
enum pot_ErrCode potDriver_readVolt(double* output);

#endif