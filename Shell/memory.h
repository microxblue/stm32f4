#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "common.h"

void write_mem (UINT32 addr, UINT32 value, int unit);
void read_mem  (UINT32 addr, UINT32 len, int unit);

#endif