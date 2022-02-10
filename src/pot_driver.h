#ifndef POTDRIVER_H
#define POTDRIVER_H

#include "a2d.h"
#include "a2dping.h"

enum a2d_ErrCode pot_readRaw(a2draw_t* output);
// Generates an a2dping_Request object. Used a2dping functions from there.
struct a2dping_Request pot_newPingRequest(const second_t timeS
											   ,const int64_t timeNS
											   ,void (*callback)(const a2draw_t output)
											   ,void (*handleErr)(void));

#endif