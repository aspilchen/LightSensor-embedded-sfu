#ifndef SAMPLER_H
#define SAMPLER_H

#include <stdint.h>

#include "global.h"
#include "cbuff.h"

#define SAMPLER_MAX_HISTORY 4095


void sampler_init(void);
void sampler_cleanup(void);
uint32_t sampler_count(void);
uint32_t sampler_capacity(void);
uint32_t sampler_size(void);
uint32_t sampler_history(volt_t* dest, uint32_t size);
uint32_t sampler_dips(void);

#endif