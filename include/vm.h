
#ifndef __VM_H__
#define __VM_H__

#include <stdint.h>
#include <inst.h>
#include <stdbool.h>

typedef struct {
	bool isHalted;
	uint64_t registers[UINT8_MAX + 1];
} VM;


void execute_program(VM* vm, const Program* program);

#endif
