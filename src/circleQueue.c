#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "circleQueue.h"

static inline queueindex_t circleQueue_increment(const struct CircleQueue* queue, const queueindex_t i)
{
	return (i + 1) % (queue->capacity);
}

void circleQueue_init(struct CircleQueue* queue, const queuesize_t size)
{
	queue->data = calloc(size+1, sizeof(queuedata_t));
	queue->top = 0;
	queue->bottom = 0;
	queue->capacity = size+1;
	queue->totalItems = 0;
	queue->size = 0;
	queue->exponentialAverage = 0;
	pthread_mutex_init(&(queue->lock), NULL);
}

// Not the most efficient but fuck it
void circleQueue_resize(struct CircleQueue* queue, const queuesize_t size)
{
	pthread_mutex_lock(&(queue->lock));
	struct CircleQueue tmp;
	queueindex_t idx = 0;
	queueindex_t mappedIdx = 0;
	tmp.data = queue->data;
	tmp.top = queue->top;
	tmp.bottom = queue->bottom;
	tmp.capacity = queue->capacity;
	tmp.exponentialAverage = queue->exponentialAverage;
	circleQueue_init(queue, size);
	mappedIdx = circleQueue_map(&tmp, idx);
	while (0 <= mappedIdx) {
		circleQueue_insert(queue, tmp.data[mappedIdx]);
		idx += 1;
		mappedIdx = circleQueue_map(&tmp, idx);
	}
	queue->totalItems = tmp.totalItems;
	queue->exponentialAverage = tmp.exponentialAverage;
	circleQueue_free(&tmp);
	pthread_mutex_unlock(&(queue->lock));
}

void circleQueue_lock(struct CircleQueue* queue)
{
	pthread_mutex_lock(&(queue->lock));
}

void circleQueue_unlock(struct CircleQueue* queue)
{
	pthread_mutex_lock(&(queue->lock));
}

queuesize_t circleQueue_size(struct CircleQueue* queue)
{
	return queue->size;
}

queuesize_t circleQueue_capacity(struct CircleQueue* queue)
{
	return queue->capacity;
}

bool circleQueue_isEmpty(struct CircleQueue* queue)
{
	return queue->top == queue->bottom;
}

queueindex_t circleQueue_map(struct CircleQueue* queue, queueindex_t i)
{
	queueindex_t tmp = (queue->top + i) % queue->capacity;
	if (i >= queue->capacity - 1) {
		return -1;
	}
	if ((queue->top < queue->bottom) && (queue->bottom <= tmp)) {
		return -1;
	}
	return tmp;
}

void circleQueue_insert(struct CircleQueue* queue, const queuedata_t value)
{
	pthread_mutex_lock(&(queue->lock));
	double weight = 0.01;
	double avg = 0;
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
	if (queue->totalItems == 0) {
		queue->exponentialAverage = value;
	} else {
		avg = (weight * value) + ((1.0-weight) * queue->exponentialAverage);
		queue->exponentialAverage = avg;
	}
	queue->totalItems += 1;
	pthread_mutex_unlock(&(queue->lock));
}

void circleQueue_free(struct CircleQueue* queue)
{
	free(queue->data);
	queue->data = NULL;
	queue->top = 0;
	queue->bottom = 0;
	queue->capacity = 0;
}