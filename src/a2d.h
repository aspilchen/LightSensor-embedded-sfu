#ifndef A2D_H
#define A2D_H

#include <stdint.h>

#define A2D_N_INFILES 7

typedef uint8_t a2dfile_t;
typedef int32_t a2draw_t;
typedef double  a2dvolt_t;

static const uint32_t  A2D_MAX_RAW  = 4095;
static const a2dvolt_t A2D_MAX_VOLT = 1.8;

enum a2d_ErrCode
{
	A2D_OK,
	A2D_ERR
};

enum a2d_ErrCode a2d_readRaw(a2draw_t* output, const a2dfile_t fileNumber);
a2dvolt_t a2d_rawToVolt(a2draw_t rawVal);

#endif