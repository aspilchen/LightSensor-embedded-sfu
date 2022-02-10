#ifndef SEGDIS_H
#define SEGDIS_H

#include <stdbool.h>

static const int32_t SEGDIS_MAX_INT = 99;
static const int32_t SEGDIS_MIN_INT = 0;


void segdis_init(void);
void segdis_setInt(const unsigned int n, const bool withDecimal);
void segdis_cleanup(void);

#endif