
#ifndef __COMPILER_H__
#define __COMPILER_H__

#define VECTOR_LOGARITHMIC_GROWTH
#include <vector.h>

#include <stdint.h>
#include <strings.h>
#include <sv.h>
#include <inst.h>
#include <hashmap.h>

#define CTX_DEBUG_FMT "%s:%lu:%lu:"
#define CTX_DEBUG(ctx) (ctx)->fileName, (ctx)->lineNo, (ctx)->index

typedef struct hashmap_s HashMap;

typedef struct {
	uint8_t reg_index;
	HashMap variables;         // map of String_View -> uint8_t register index

	uint16_t constants_index;
	HashMap constants;         // map of const char* -> uint16_t index in constants table

	uint16_t strings_index;
	HashMap strings;           // map of String_View -> uint16_t index in strings table

	HashMap labels;            // map of String_View -> uint64_t instruction index
	HashMap unresolvedLabels;  // map of String_View -> array of UnresolvedSymbol containg all positions where the label is used


	Program* program;
	// Debug info to refer to when an error is encountered
	const char* fileName;
	uint64_t lineNo;
	uint64_t index;
} Context;

typedef enum {
	NONE,
	CONST_I,
	CONST_F,
	VAR,
	LABEL,
	STR,
} OperandType;

typedef struct {
	OperandType type;
	union {
		int64_t integer;
		double floating;
		String_View variable;
		String_View label;
		String_View string;
	};

	// Debug info to refer to when an error is encountered
	const char* fileName;
	uint64_t lineNo;
	uint64_t index;
} Operand;

typedef enum {
	IMPLICIT,
	CONST,
	REG,
	STRING,
} InstVariant;

typedef struct {
	uint64_t instIndex;
	Operand operand;
	uint8_t argIndex;
} UnresolvedSymbol;

Program* compile_file(const char* file_path);

void write_prog_to_file(const char* file_path, const Program* program);

const char* describe_operand_type(OperandType type);

#endif
