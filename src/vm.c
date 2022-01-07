#include <stdio.h>
#include <vm.h>
#include <assert.h>
#include <executors.h>

void load_program_in_vm(VM* vm, Inst* inst, size_t instCount) {
	vm->inst = inst;
	vm->instCount = instCount;
}

VM vm;

void execute_program(VM* vm) {
	while (!vm->isHalted) {
		Inst inst = vm->inst[vm->ip];
		printf("\t%s\n", describe(&inst));

		(*executors[inst.op])(vm, inst.args);

		assert(vm->ip < vm->instCount);
	}
}

int main() {
	Inst fib[] = {
		create_inst2(OP_LOAD_CONST, 0, 0), 			// load 0 at `0

		create_inst2(OP_LOAD_CONST, 1, 0), 			// load 0 at `1
		create_inst2(OP_LOAD_CONST, 2, 1), 			// load 1 at `2
		create_inst1(OP_DEBUG_PRINT, 1), 			// print contents of `1
		create_inst1(OP_DEBUG_PRINT, 2), 			// print contents of `2

		create_inst3(OP_ADD_REG, 3, 1, 2),     			// add num from `1 and `2 and store at `3
		create_inst2(OP_LOAD_REG, 1, 2),   			// load `1 with `2
		create_inst2(OP_LOAD_REG, 2, 3),   			// load `2 with `3
		create_inst1(OP_DEBUG_PRINT, 3), 			// print contents of `3

		create_inst3(OP_ADD_CONST, 0, 0, 1),			// add 1 to `0 and store at `0
		create_inst3(OP_EQ_CONST, 4, 0, 15), 			// check if `0 == 15 and store in `4
		create_inst3(OP_CMP_JMP_CONST, 4, 1, -6), 		// if `4 is true, move to next line else go back 6 lines
		create_inst0(OP_HALT)
	};

	load_program_in_vm(&vm, fib, 14);
	execute_program(&vm);
	return 0;
}


