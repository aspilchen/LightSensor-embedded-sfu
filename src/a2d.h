#ifndef A2D_H
#define A2D_H

/*
 * a2d is a simple helper to read values from an A2D pin. If you know the 
 * pin/file number it removes the need to manage files and paths.
 */

#include <stdint.h>

#include "global.h"

#define A2D_MAX_RAW 4095
#define A2D_MAX_VOLT 1.8
#define A2D_N_INFILES 7

typedef uint8_t a2dfile_t;

enum a2d_ErrCode
{
	A2D_OK,
	A2D_ERR
};

enum a2d_ErrCode a2d_readRaw(raw_t* output, const a2dfile_t fileNumber);
volt_t a2d_rawToVolt(raw_t rawVal);

#endif