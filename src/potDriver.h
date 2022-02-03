#ifndef POTDRIVER_H
#define POTDRIVER_H

#include <stdint.h>

enum pot_ErrCode {
	POT_ERROR = -3,
	POT_FOPEN_ERR,
	POT_READ_ERR,
	POT_OK

};

int32_t potDriver_readRaw();
int64_t potDriver_readVolt();

#endif