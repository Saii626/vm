
#ifndef __EXECUTORS_H__
#define __EXECUTORS_H__

#include <stdint.h>
#include <vm.h>
#include <inst.h>

typedef void (*executor)(VM*, uint8_t[3]);

extern executor executors[OPS_COUNT];

#endif
