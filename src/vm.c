#define VECTOR_LOGARITHMIC_GROWTH
#include <vector.h>
#include <stdio.h>
#include <vm.h>
#include <assert.h>
#include <errno.h>
#include "./bytecode_parser.c"

VM vm;


#define NEXT_INSTRUCTION \
\
_Pragma("GCC diagnostic push")						\
_Pragma("GCC diagnostic ignored \"-Wgnu-label-as-value\"")		\
        currInst = instructions[ip]; 					\
        goto *jump_table[currInst.op]; 					\
_Pragma("GCC diagnostic pop") 						\


void execute_program(VM* vm, const Program* program) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-label-as-value"
	static void* jump_table[] = { 
		&&noop,

		&&load_implicit, &&load_const, &&load_reg, &&load_str,

		&&jmp_implicit, &&jmp_const, &&jmp_reg,
		&&jmp_ze_implicit, &&jmp_ze_const, &&jmp_ze_reg,
		&&jmp_nz_implicit, &&jmp_nz_const, &&jmp_nz_reg,
		&&jmp_gt_implicit, &&jmp_gt_const, &&jmp_gt_reg,
		&&jmp_lt_implicit, &&jmp_lt_const, &&jmp_lt_reg,
		&&jmp_ge_implicit, &&jmp_ge_const, &&jmp_ge_reg,
		&&jmp_le_implicit, &&jmp_le_const, &&jmp_le_reg,

		&&sadd_implicit, &&sadd_const, &&sadd_reg,
		&&ssub_implicit, &&ssub_const, &&ssub_reg,
		&&smul_implicit, &&smul_const, &&smul_reg,
		&&sdiv_implicit, &&sdiv_const, &&sdiv_reg,

		&&uadd_implicit, &&uadd_const, &&uadd_reg,
		&&usub_implicit, &&usub_const, &&usub_reg,
		&&umul_implicit, &&umul_const, &&umul_reg,
		&&udiv_implicit, &&udiv_const, &&udiv_reg,

		&&add_implicit, &&add_const, &&add_reg,
		&&sub_implicit, &&sub_const, &&sub_reg,
		&&mul_implicit, &&mul_const, &&mul_reg,
		&&div_implicit, &&div_const, &&div_reg,

		&&debug_string, &&debug_reg,

		&&halt
	};
#pragma GCC diagnostic pop
	
	register uint64_t ip = 0;
	const Inst* instructions = program->instructions;
	Inst currInst;

	NEXT_INSTRUCTION;

noop:
	ip += 1;
	NEXT_INSTRUCTION;


// Loads
load_implicit:
	vm->registers[currInst.args[0]] = (uint64_t)((int16_t) UINT16_ARG(currInst.args[1], currInst.args[2]));
	ip += 1;
	NEXT_INSTRUCTION;

load_const:
	vm->registers[currInst.args[0]] = program->constants[UINT16_ARG(currInst.args[1], currInst.args[2])];
	ip += 1;
	NEXT_INSTRUCTION;

load_reg:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]];
	ip += 1;
	NEXT_INSTRUCTION;

load_str:
	vm->registers[currInst.args[0]] = (uint64_t) &program->strings[UINT16_ARG(currInst.args[1], currInst.args[2])];
	ip += 1;
	NEXT_INSTRUCTION;


// Unconditional jumps
jmp_implicit:
	{
		uint16_t diff = UINT16_ARG(currInst.args[0], currInst.args[1]);
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&diff);
	}
	NEXT_INSTRUCTION;

jmp_const:
	ip = (uint64_t)((int64_t)ip + *(int16_t*)&program->constants[UINT16_ARG(currInst.args[0], currInst.args[1])]);
	NEXT_INSTRUCTION;

jmp_reg:
	ip = (uint64_t)((int64_t)ip + *(int16_t*)&vm->registers[currInst.args[0]]);
	NEXT_INSTRUCTION;


// Jump if zero
jmp_ze_implicit:
	if (vm->registers[currInst.args[0]] == 0) {
		uint16_t diff = UINT16_ARG(currInst.args[1], currInst.args[2]);
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&diff);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_ze_const:
	if (vm->registers[currInst.args[0]] == 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&program->constants[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_ze_reg:
	if (vm->registers[currInst.args[0]] == 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&vm->registers[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Jump if not zero
jmp_nz_implicit:
	if (vm->registers[currInst.args[0]] != 0) {
		uint16_t diff = UINT16_ARG(currInst.args[1], currInst.args[2]);
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&diff);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_nz_const:
	if (vm->registers[currInst.args[0]] != 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&program->constants[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_nz_reg:
	if (vm->registers[currInst.args[0]] != 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&vm->registers[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Jump if greater than 0
jmp_gt_implicit:
	if ((int64_t)vm->registers[currInst.args[0]] > 0) {
		uint16_t diff = UINT16_ARG(currInst.args[1], currInst.args[2]);
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&diff);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_gt_const:
	if ((int64_t)vm->registers[currInst.args[0]] > 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&program->constants[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_gt_reg:
	if ((int64_t)vm->registers[currInst.args[0]] > 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&vm->registers[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Jump if less than 0
jmp_lt_implicit:
	if ((int64_t)vm->registers[currInst.args[0]] < 0) {
		uint16_t diff = UINT16_ARG(currInst.args[1], currInst.args[2]);
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&diff);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_lt_const:
	if ((int64_t)vm->registers[currInst.args[0]] < 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&program->constants[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_lt_reg:
	if ((int64_t)vm->registers[currInst.args[0]] < 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&vm->registers[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Jump if greater than or equal to 0
jmp_ge_implicit:
	if ((int64_t)vm->registers[currInst.args[0]] >= 0) {
		uint16_t diff = UINT16_ARG(currInst.args[1], currInst.args[2]);
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&diff);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_ge_const:
	if ((int64_t)vm->registers[currInst.args[0]] >= 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&program->constants[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_ge_reg:
	if ((int64_t)vm->registers[currInst.args[0]] >= 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&vm->registers[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Jump if less than or equal to 0
jmp_le_implicit:
	if ((int64_t)vm->registers[currInst.args[0]] <= 0) {
		uint16_t diff = UINT16_ARG(currInst.args[1], currInst.args[2]);
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&diff);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_le_const:
	if ((int64_t)vm->registers[currInst.args[0]] <= 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&program->constants[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;

jmp_le_reg:
	if ((int64_t)vm->registers[currInst.args[0]] <= 0) {
		ip = (uint64_t)((int64_t)ip + *(int16_t*)&vm->registers[UINT16_ARG(currInst.args[1], currInst.args[2])]);
	} else {
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Signed addition
sadd_implicit:
	{
		int64_t add = *(int64_t*)&(vm->registers[currInst.args[1]]) + *(int8_t*)&currInst.args[2];
		vm->registers[currInst.args[0]] = *(uint64_t*)&add;
		ip += 1;
	}
	NEXT_INSTRUCTION;

sadd_const:
	{
		int64_t add = *(int64_t*)&(vm->registers[currInst.args[1]]) + *(int64_t*)&(program->constants[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&add;
		ip += 1;
	}
	NEXT_INSTRUCTION;

sadd_reg:
	{
		int64_t add = *(int64_t*)&(vm->registers[currInst.args[1]]) + *(int64_t*)&(vm->registers[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&add;
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Signed subtraction
ssub_implicit:
	{
		int64_t sub = *(int64_t*)&(vm->registers[currInst.args[1]]) - *(int8_t*)&currInst.args[2];
		vm->registers[currInst.args[0]] = *(uint64_t*)&sub;
		ip += 1;
	}
	NEXT_INSTRUCTION;

ssub_const:
	{
		int64_t sub = *(int64_t*)&(vm->registers[currInst.args[1]]) - *(int64_t*)&(program->constants[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&sub;
		ip += 1;
	}
	NEXT_INSTRUCTION;

ssub_reg:
	{
		int64_t sub = *(int64_t*)&(vm->registers[currInst.args[1]]) - *(int64_t*)&(vm->registers[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&sub;
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Signed multiply
smul_implicit:
	{
		int64_t mul = *(int64_t*)&(vm->registers[currInst.args[1]]) * *(int8_t*)&currInst.args[2];
		vm->registers[currInst.args[0]] = *(uint64_t*)&mul;
		ip += 1;
	}
	NEXT_INSTRUCTION;

smul_const:
	{
		int64_t mul = *(int64_t*)&(vm->registers[currInst.args[1]]) * *(int64_t*)&(program->constants[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&mul;
		ip += 1;
	}
	NEXT_INSTRUCTION;

smul_reg:
	{
		int64_t mul = *(int64_t*)&(vm->registers[currInst.args[1]]) * *(int64_t*)&(vm->registers[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&mul;
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Signed divide
sdiv_implicit:
	{
		int64_t div = *(int64_t*)&(vm->registers[currInst.args[1]]) / *(int8_t*)&currInst.args[2];
		vm->registers[currInst.args[0]] = *(uint64_t*)&div;
		ip += 1;
	}
	NEXT_INSTRUCTION;

sdiv_const:
	{
		int64_t div = *(int64_t*)&(vm->registers[currInst.args[1]]) / *(int64_t*)&(program->constants[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&div;
		ip += 1;
	}
	NEXT_INSTRUCTION;

sdiv_reg:
	{
		int64_t div = *(int64_t*)&(vm->registers[currInst.args[1]]) / *(int64_t*)&(vm->registers[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&div;
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Unsigned addition
uadd_implicit:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] + currInst.args[2];
	ip += 1;
	NEXT_INSTRUCTION;

uadd_const:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] + program->constants[currInst.args[2]];
	ip += 1;
	NEXT_INSTRUCTION;

uadd_reg:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] + vm->registers[currInst.args[2]];
	ip += 1;
	NEXT_INSTRUCTION;


// Unsigned subtraction
usub_implicit:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] - currInst.args[2];
	ip += 1;
	NEXT_INSTRUCTION;

usub_const:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] - program->constants[currInst.args[2]];
	ip += 1;
	NEXT_INSTRUCTION;

usub_reg:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] - vm->registers[currInst.args[2]];
	ip += 1;
	NEXT_INSTRUCTION;


// Unsigned multiply
umul_implicit:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] * currInst.args[2];
	ip += 1;
	NEXT_INSTRUCTION;

umul_const:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] * program->constants[currInst.args[2]];
	ip += 1;
	NEXT_INSTRUCTION;

umul_reg:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] * vm->registers[currInst.args[2]];
	ip += 1;
	NEXT_INSTRUCTION;


// Unsigned divide
udiv_implicit:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] / currInst.args[2];
	ip += 1;
	NEXT_INSTRUCTION;

udiv_const:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] / program->constants[currInst.args[2]];
	ip += 1;
	NEXT_INSTRUCTION;

udiv_reg:
	vm->registers[currInst.args[0]] = vm->registers[currInst.args[1]] / vm->registers[currInst.args[2]];
	ip += 1;
	NEXT_INSTRUCTION;


// Floating point addition
add_implicit:
	{
		double add = *(double*)&(vm->registers[currInst.args[1]]) + *(double*)&currInst.args[2];
		vm->registers[currInst.args[0]] = *(uint64_t*)&add;
		ip += 1;
	}
	NEXT_INSTRUCTION;

add_const:
	{
		double add = *(double*)&(vm->registers[currInst.args[1]]) + *(double*)&(program->constants[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&add;
		ip += 1;
	}
	NEXT_INSTRUCTION;

add_reg:
	{
		double add = *(double*)&(vm->registers[currInst.args[1]]) + *(double*)&(vm->registers[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&add;
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Floating point substraction
sub_implicit:
	{
		double sub = *(double*)&(vm->registers[currInst.args[1]]) - *(double*)&currInst.args[2];
		vm->registers[currInst.args[0]] = *(uint64_t*)&sub;
		ip += 1;
	}
	NEXT_INSTRUCTION;

sub_const:
	{
		double sub = *(double*)&(vm->registers[currInst.args[1]]) - *(double*)&(program->constants[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&sub;
		ip += 1;
	}
	NEXT_INSTRUCTION;

sub_reg:
	{
		double sub = *(double*)&(vm->registers[currInst.args[1]]) - *(double*)&(vm->registers[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&sub;
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Floating point multiply
mul_implicit:
	{
		double mul = *(double*)&(vm->registers[currInst.args[1]]) * *(double*)&currInst.args[2];
		vm->registers[currInst.args[0]] = *(uint64_t*)&mul;
		ip += 1;
	}
	NEXT_INSTRUCTION;

mul_const:
	{
		double mul = *(double*)&(vm->registers[currInst.args[1]]) - *(double*)&(program->constants[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&mul;
		ip += 1;
	}
	NEXT_INSTRUCTION;

mul_reg:
	{
		double mul = *(double*)&(vm->registers[currInst.args[1]]) - *(double*)&(vm->registers[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&mul;
		ip += 1;
	}
	NEXT_INSTRUCTION;

// Floating point divide
div_implicit:
	{
		double div = *(double*)&(vm->registers[currInst.args[1]]) / *(double*)&currInst.args[2];
		vm->registers[currInst.args[0]] = *(uint64_t*)&div;
		ip += 1;
	}
	NEXT_INSTRUCTION;

div_const:
	{
		double div = *(double*)&(vm->registers[currInst.args[1]]) / *(double*)&(program->constants[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&div;
		ip += 1;
	}
	NEXT_INSTRUCTION;

div_reg:
	{
		double div = *(double*)&(vm->registers[currInst.args[1]]) / *(double*)&(vm->registers[currInst.args[2]]);
		vm->registers[currInst.args[0]] = *(uint64_t*)&div;
		ip += 1;
	}
	NEXT_INSTRUCTION;


// Debug print
debug_string:
	{
		String* s = (String*) vm->registers[currInst.args[0]];
		fprintf(stderr, "%.*s\n", (int)s->len, s->str);
		ip += 1;
	}
	NEXT_INSTRUCTION;

debug_reg:
	{
		uint64_t contents = vm->registers[currInst.args[0]];
		fprintf(stderr, "unsigned: %lu, signed:%li, float: %lf\n", contents, *(int64_t*)&contents, *(double*)&contents);
		ip += 1;
	}
	NEXT_INSTRUCTION;

halt:
	fprintf(stderr, "\n~~~PROGRAM ENDED~~~\n");
	exit(0);
}


int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s <file_path.vm>\n", argv[0]);
		exit(1);
	}

	Program* p = load_program_from_file(argv[1]);
	execute_program(&vm, p);

	return 0;
}


