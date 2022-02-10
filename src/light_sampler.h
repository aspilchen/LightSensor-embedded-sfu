#ifndef SAMPLER_H
#define SAMPLER_H

#include <stdint.h>
#include "circleQueue.h"

#define SAMPLER_MAX_HISTORY 4090

typedef queuedata_t lightsample_t;
typedef queuesize_t samplesize_t;

void sampler_init(void);
samplesize_t sampler_count(void);
samplesize_t sampler_capacity(void);
samplesize_t sampler_size(void);
samplesize_t sampler_history(lightsample_t* dest, samplesize_t size);  //-- display the full sample history being saved.
void sampler_get(lightsample_t* dest, const samplesize_t size, const samplesize_t n);
uint32_t sampler_dips(void); //-- display number of dips.
void sampler_cleanup(void);

#endif