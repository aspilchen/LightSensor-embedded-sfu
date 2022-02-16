#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cbuff.h"

static void cbuff_initImpl(struct CBuff* buff, const uint32_t capacity);
static void cbuff_destroyImpl(struct CBuff* buff);
static void cbuff_insertImpl(struct CBuff* dest, const volt_t value);
static void cbuff_resizeImpl(struct CBuff* buff, const uint32_t size);
static uint32_t cbuff_dumpImpl(volt_t* dest, const struct CBuff* src, const uint32_t size);
static uint32_t cbuff_sizeImpl(const struct CBuff* buff);
static uint32_t cbuff_capacityImpl(const struct CBuff* buff);
static void cbuff_moveLossy(struct CBuff* dest, struct CBuff* src);
static void cbuff_moveLossless(struct CBuff* dest, struct CBuff* src);
static inline index_t cbuff_increment(const struct CBuff* buff, const index_t i);

void cbuff_init(struct CBuff* buff)
{
	const uint32_t capacity = 100;
	cbuff_initImpl(buff, capacity);
	pthread_mutex_init(&(buff->lock), NULL);
}

void cbuff_destroy(struct CBuff* buff)
{
	cbuff_destroyImpl(buff);
	pthread_mutex_destroy(&(buff->lock));
}

void cbuff_insert(struct CBuff* dest, const volt_t value)
{
	pthread_mutex_lock(&(dest->lock));
	cbuff_insertImpl(dest, value);
	pthread_mutex_unlock(&(dest->lock));
}

void cbuff_resize(struct CBuff* buff, const uint32_t capacity)
{
	pthread_mutex_lock(&(buff->lock));
	cbuff_resizeImpl(buff, capacity);
	pthread_mutex_unlock(&(buff->lock));
}

uint32_t cbuff_dump(volt_t* dest, struct CBuff* src, const uint32_t size)
{
	pthread_mutex_lock(&(src->lock));
	int32_t sampleSize = cbuff_dumpImpl(dest, src, size);
	pthread_mutex_unlock(&(src->lock));
	return sampleSize;
}

uint32_t cbuff_size(const struct CBuff* buff)
{
	return cbuff_sizeImpl(buff);
}

uint32_t cbuff_capacity(const struct CBuff* buff)
{
	return cbuff_capacityImpl(buff);
}

static void cbuff_initImpl(struct CBuff* buff, const uint32_t capacity)
{
	buff->data = malloc(sizeof(*(buff->data)) * (capacity + 1));
	buff->top = 0;
	buff->bottom = 0;
	buff->capacity = capacity+1;
	buff->size = 0;
}

static void cbuff_destroyImpl(struct CBuff* buff)
{
	if (buff->data) {
		free(buff->data);
		buff->data = NULL;
	}
	buff->top = 0;
	buff->bottom = 0;
	buff->capacity = 0;
	buff->size = 0;
}

static void cbuff_insertImpl(struct CBuff* dest, const volt_t value)
{
	index_t bot = dest->bottom;
	index_t top = dest->top;
	dest->data[bot] = value;
	bot = cbuff_increment(dest, bot);
	dest->bottom = bot;
	if (bot == top) {
		top = cbuff_increment(dest, top);
		dest->top = top;
	} else {
		dest->size += 1;
	}
}

static void cbuff_resizeImpl(struct CBuff* buff, const uint32_t capacity)
{
	struct CBuff tmp;
	cbuff_moveLossless(&tmp, buff);
	cbuff_initImpl(buff, capacity);
	cbuff_moveLossy(buff, &tmp);
	cbuff_destroyImpl(&tmp);
}

static uint32_t cbuff_dumpImpl(volt_t* dest, const struct CBuff* src, const uint32_t size)
{
	uint32_t itemsRead = 0;
	index_t destIdx = 0;
	index_t srcIdx = src->top;
	while (srcIdx != src->bottom) {
		dest[destIdx] = src->data[srcIdx];
		destIdx += 1;
		itemsRead += 1;
		srcIdx = cbuff_increment(src, srcIdx);
	}
	return itemsRead;
}

static uint32_t cbuff_sizeImpl(const struct CBuff* buff)
{
	return buff->size;
}

static uint32_t cbuff_capacityImpl(const struct CBuff* buff)
{
	return buff->capacity - 1;
}

static void cbuff_moveLossy(struct CBuff* dest, struct CBuff* src)
{
	index_t srcIdx = src->top;
	while (srcIdx != src->bottom) {
		cbuff_insertImpl(dest, src->data[srcIdx]);
		srcIdx = cbuff_increment(src, srcIdx);
	}
}

static void cbuff_moveLossless(struct CBuff* dest, struct CBuff* src)
{
	dest->data = src->data;
	dest->top = src->top;
	dest->bottom = src->bottom;
	dest->capacity = src->capacity;
	dest->size = src->size;
}

static inline index_t cbuff_increment(const struct CBuff* buff, const index_t i)
{
	return (i + 1) % (buff->capacity);
}
