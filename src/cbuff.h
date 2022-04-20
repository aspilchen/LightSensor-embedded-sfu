#ifndef CBUFF_H
#define CBUFF_H

#include <stdint.h>
#include <stdbool.h>

#include "global.h"

struct CBuff
{
	volt_t* data;
	index_t top;
	index_t bottom;
	uint32_t size;
	uint32_t capacity;
	pthread_mutex_t lock;
};

void cbuff_init(struct CBuff* buff);
void cbuff_destroy(struct CBuff* buff);
void cbuff_insert(struct CBuff* buff, const volt_t value);
void cbuff_resize(struct CBuff* buff, const uint32_t size);
uint32_t cbuff_size(const struct CBuff* buff);
uint32_t cbuff_capacity(const struct CBuff* buff);
uint32_t cbuff_dump(volt_t* dest, struct CBuff* src, const uint32_t size);

#endif