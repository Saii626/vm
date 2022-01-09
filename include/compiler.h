
#ifndef __COMPILER_H__
#define __COMPILER_H__

#define VECTOR_LOGARITHMIC_GROWTH
#include <vector.h>

#include <stdint.h>
#include <sv.h>
#include <inst.h>
#include <hashmap.h>

#define CTX_DEBUG_FMT "%s:%lu:%lu:"
#define CTX_DEBUG(ctx) (ctx)->fileName, (ctx)->lineNo, (ctx)->index

typedef struct hashmap_s HashMap;

typedef struct {
	uint8_t reg_index;
	HashMap labels;            // map of String_View -> instruction index
	HashMap variables;         // map of String_View -> uint8_t
	HashMap unresolvedLabels;  // map of String_View -> array of uint8_t* containg all positions where the label is used

	// Debug info to refer to when an error is encountered
	const char* fileName;
	uint64_t lineNo;
	uint64_t index;
} Context;

typedef enum {
	NONE,
	CONST,
	VAR,
	LABEL,
} OperandType;

typedef struct {
	OperandType type;
	union {
		uint64_t constant;
		String_View variable;
		String_View label;
	};

	// Debug info to refer to when an error is encountered
	uint64_t lineNo;
	uint64_t index;
} Operand;

Program compile_file(const char* file_path);

void compile_noop(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3);
void compile_load(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3);
void compile_jmp(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3);
void compile_cmp_jmp(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3);
void compile_eq(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3);
void compile_add(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3);
void compile_debug_print(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3);
void compile_halt(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3);

void resolve_all_symbols(Context* state, Program* program);

void write_prog_to_file(const char* file_path, const char* disasm_path, const Program* program);

const char* describe_operand_type(OperandType type);

#endif
