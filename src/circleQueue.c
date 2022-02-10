#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "circleQueue.h"

static void circleQueue_reset(struct CircleQueue* queue, const queuesize_t capacity);
static inline queueindex_t circleQueue_increment(const struct CircleQueue* queue, const queueindex_t i);

void circleQueue_init(struct CircleQueue* queue)
{
	const queuesize_t initCapacity = 100;
	queue->data = calloc(initCapacity+1, sizeof(queuedata_t));
	queue->top = 0;
	queue->bottom = 0;
	queue->capacity = initCapacity+1;
	queue->totalItems = 0;
	queue->size = 0;
	pthread_mutex_init(&(queue->lock), NULL);
}

// Not the most efficient but fuck it
void circleQueue_resize(struct CircleQueue* queue, const queuesize_t capacity)
{
	pthread_mutex_lock(&(queue->lock));
	queueindex_t idx = 0;
	queueindex_t mappedIdx = 0;
	struct CircleQueue tmp;
	tmp.data = queue->data;
	tmp.top = queue->top;
	tmp.bottom = queue->bottom;
	tmp.capacity = queue->capacity;
	tmp.totalItems = queue->totalItems;
	circleQueue_reset(queue, capacity);
	mappedIdx = circleQueue_map(&tmp, idx);
	while (0 <= mappedIdx) {
		circleQueue_insert(queue, tmp.data[mappedIdx], false);
		idx += 1;
		mappedIdx = circleQueue_map(&tmp, idx);
	}
	queue->totalItems = tmp.totalItems;
	circleQueue_free(&tmp);
	pthread_mutex_unlock(&(queue->lock));
}

queuesize_t circleQueue_dump(queuedata_t* dest, queuesize_t bufferSize, struct CircleQueue* src)
{
	pthread_mutex_lock(&(src->lock));
	queuesize_t itemsRead = 0;
	queueindex_t buffIdx = 0;
	queueindex_t qIdx = 0;
	queueindex_t mappedIdx = circleQueue_map(src, qIdx);
	if (bufferSize < src->size) {
		int32_t diff = src->size - bufferSize;
		qIdx += diff;
		mappedIdx = circleQueue_map(src, qIdx);
	}
	while (0 <= mappedIdx) {
		dest[buffIdx] = src->data[mappedIdx];
		buffIdx += 1;
		qIdx += 1;
		mappedIdx = circleQueue_map(src, qIdx);
		itemsRead += 1;
	}
	pthread_mutex_unlock(&(src->lock));
	return itemsRead;
}

queuesize_t circleQueue_size(struct CircleQueue* queue)
{
	return queue->size;
}

queuesize_t circleQueue_capacity(struct CircleQueue* queue)
{
	return queue->capacity - 1;
}

bool circleQueue_isEmpty(struct CircleQueue* queue)
{
	return queue->top == queue->bottom;
}

queueindex_t circleQueue_map(struct CircleQueue* queue, int32_t i)
{
	queueindex_t tmp = (queue->top + i) % queue->capacity;
	if (queue->capacity < i) {
		return -1;
	}
	if (tmp == queue->bottom) {
		return -1;
	}
	return tmp;
}

void circleQueue_insert(struct CircleQueue* queue, const queuedata_t value, const bool lock)
{
	if (lock) {
		pthread_mutex_lock(&(queue->lock));
	}
	queueindex_t bot = queue->bottom;
	queueindex_t top = queue->top;
	queue->data[bot] = value;
	bot = circleQueue_increment(queue, bot);
	queue->bottom = bot;
	if (bot == top) {
		top = circleQueue_increment(queue, top);
		queue->top = top;
	} else {
		queue->size += 1;
	}

	queue->totalItems += 1;
	if (lock) {
		pthread_mutex_unlock(&(queue->lock));
	}
}

void circleQueue_free(struct CircleQueue* queue)
{
	free(queue->data);
	queue->data = NULL;
	queue->top = 0;
	queue->bottom = 0;
	queue->capacity = 0;
}

void circleQueue_lock(struct CircleQueue* queue)
{
	pthread_mutex_lock(&(queue->lock));
}

void circleQueue_unlock(struct CircleQueue* queue)
{
	pthread_mutex_unlock(&(queue->lock));
}

static inline queueindex_t circleQueue_increment(const struct CircleQueue* queue, const queueindex_t i)
{
	return (i + 1) % (queue->capacity);
}

static void circleQueue_reset(struct CircleQueue* queue, const queuesize_t capacity)
{
	queue->data = calloc(capacity+1, sizeof(queuedata_t));
	queue->top = 0;
	queue->bottom = 0;
	queue->capacity = capacity+1;
	queue->totalItems = 0;
	queue->size = 0;
}