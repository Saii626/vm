
#ifndef __VM_H__
#define __VM_H__

#include <stdint.h>
#include <inst.h>
#include <stdbool.h>

typedef struct {
	Inst* inst;
	size_t instCount;
	uint64_t ip;
	bool isHalted;

	uint64_t registers[256];
} VM;

void load_program_in_vm(VM* vm, Inst* inst, size_t instCount);

void execute_program(VM* vm);

#endif
