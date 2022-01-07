
#include <stdio.h>
#include <executors.h>

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

