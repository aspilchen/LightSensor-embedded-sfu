#ifndef CIRCLEQ_H
#define CIRCLEQ_H

#include <stdint.h>
#include <stdbool.h>

// Used to maintain types across all functions. If I want to change a type (like the data) I can do it here.
typedef double queuedata_t;
typedef int32_t queueindex_t;
typedef uint32_t queuesize_t;

struct CircleQueue
{
	queuedata_t* data;
	queueindex_t top;
	queueindex_t bottom;
	queuesize_t size;
	queuesize_t capacity;
	queuesize_t totalItems;
	double exponentialAverage;
	pthread_mutex_t lock;
};

void circleQueue_init(struct CircleQueue* queue, const queuesize_t size);
void circleQueue_lock(struct CircleQueue* queue);
void circleQueue_unlock(struct CircleQueue* queue);
void circleQueue_resize(struct CircleQueue* queue, const queuesize_t size);
queuesize_t circleQueue_size(struct CircleQueue* queue);
queuesize_t circleQueue_capacity(struct CircleQueue* queue);
queueindex_t circleQueue_currentIdx(struct CircleQueue* queue);
queueindex_t circleQueue_map(struct CircleQueue* queue, queueindex_t i);
void circleQueue_insert(struct CircleQueue* queue, const queuedata_t value);
void circleQueue_free(struct CircleQueue* queue);

#endif