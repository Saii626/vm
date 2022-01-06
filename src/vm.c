#include <stdio.h>
#include <vm.h>
#include <assert.h>

static void execute_noop(VM* vm, uint8_t* args) {
	(void) args;
	vm->ip += 1;
}

static void execute_load_const(VM* vm, uint8_t* args) {
	vm->registers[args[0]] = args[1];
	vm->ip += 1;
}

static void execute_load_reg(VM* vm, uint8_t* args) {
	vm->registers[args[0]] = vm->registers[args[1]];
	vm->ip += 1;
}

static void execute_add_const(VM* vm, uint8_t* args) {
	vm->registers[args[0]] = vm->registers[args[1]] + args[2];
	vm->ip += 1;
}

static void execute_add_reg(VM* vm, uint8_t* args) {
	vm->registers[args[0]] = vm->registers[args[1]] + vm->registers[args[2]];
	vm->ip += 1;
}

static void execute_eq_const(VM* vm, uint8_t* args) {
	vm->registers[args[0]] = vm->registers[args[1]] == args[2];
	vm->ip += 1;
}

static void execute_eq_reg(VM* vm, uint8_t* args) {
	vm->registers[args[0]] = vm->registers[args[1]] == vm->registers[args[2]];
	vm->ip += 1;
}

static void execute_debug_print(VM* vm, uint8_t* args) {
	printf("reg_%i = %lu\n", args[0], vm->registers[args[0]]);
	vm->ip += 1;
}

static void execute_halt(VM* vm, uint8_t* args) {
	vm->isHalted = true;
}

static void execute_jmp_const(VM* vm, uint8_t* args) {
	vm->ip += (int) args[0];
}

static void execute_jmp_reg(VM* vm, uint8_t* args) {
	vm->ip += (int) vm->registers[args[0]];
}

static void execute_cmp_jmp_const(VM* vm, uint8_t* args) {
	vm->ip = (uint64_t) ((int64_t)vm->ip + (((bool)vm->registers[args[0]]) ? (int8_t)args[1] : (int8_t)args[2]));
}

static void execute_cmp_jmp(VM* vm, uint8_t* args) {
	vm->ip += (int) (((bool)vm->registers[args[0]]) ? vm->registers[args[1]] : vm->registers[args[2]]);
}

void load_program_in_vm(VM* vm, Inst* instructions, size_t instCount) {
	vm->instructions = instructions;
	vm->instCount = instCount;
}

VM vm;
executor executors[OPS_COUNT] = {
	execute_noop,

	execute_load_const,
	execute_load_reg,

	execute_jmp_const,
	execute_jmp_reg,

	execute_cmp_jmp_const,
	execute_cmp_jmp,

	execute_eq_const,
	execute_eq_reg,

	execute_add_const,
	execute_add_reg,

	execute_debug_print,
	execute_halt
};

void execute_program(VM* vm) {
	while (!vm->isHalted) {
		Inst inst = vm->instructions[vm->ip];
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


