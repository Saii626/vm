
#ifndef __VM_H__
#define __VM_H__

#include <stdint.h>
#include <inst.h>
#include <stdbool.h>

typedef struct {
	Program program;
	uint64_t ip;
	bool isHalted;

	uint64_t registers[UINT8_MAX + 1];
} VM;

Program load_program_from_file(const char* file_path);

void execute_program(VM* vm, Program program);

#endif
