#ifndef SEGDIS_H
#define SEGDIS_H

#include <stdbool.h>

void segdis_init(void);
void segdis_setInt(const unsigned int n, const bool withDecimal);
void segdis_cleanup(void);

#endif