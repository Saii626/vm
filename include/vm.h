
#include <stdint.h>
#include <instructions.h>
#include <stdbool.h>

typedef struct {
	Inst* instructions;
	size_t instCount;
	uint64_t ip;
	bool isHalted;

	uint64_t registers[256];
} VM;

typedef void (*executor)(VM*, uint8_t[3]);

void load_program_in_vm(VM* vm, Inst* instructions, size_t instCount);

void execute_program(VM* vm);
