

#ifndef __INST_H__
#define __INST_H__

#include <stdint.h>
#include <stdio.h>
#include <vector.h>

typedef struct {
	uint8_t op;
	uint8_t args[3];
} Inst;

typedef struct {
	uint64_t instructionsCount;
	uint64_t constantsCount;
	uint64_t stringsCount;
	uint64_t stringsSize;
} ProgramHeader;

typedef struct {
	const char* str;
	size_t len;
} String;

typedef struct {
	ProgramHeader header;
	Inst* instructions;
	uint64_t constants[UINT16_MAX + 1];
	String strings[UINT16_MAX + 1];
} Program;

typedef enum {
	OP_NOOP = 0,
	// ------------------------------------------------------------------	
	
	/**
	 * Implicit ops have the operand directly embedded in the instruction
	 * Const ops have an operand refer to constants table
	 * Reg ops have all their operands in registers
	 */
	
	// Load value to a register
	OP_LOAD_IMPLICIT,
	OP_LOAD_CONST,
	OP_LOAD_REG,
	// ------------------------------------------------------------------	


	// Unconditional jump
	OP_JMP_IMPLICIT,
	OP_JMP_CONST,
	OP_JMP_REG,
	// ------------------------------------------------------------------	

	// Jump if 0
	OP_JMP_ZE_IMPLICIT,
	OP_JMP_ZE_CONST,
	OP_JMP_ZE_REG,
	// ------------------------------------------------------------------	
	
	// Jump if not 0
	OP_JMP_NZ_IMPLICIT,
	OP_JMP_NZ_CONST,
	OP_JMP_NZ_REG,
	// ------------------------------------------------------------------	

	// Jump if greater than 0
	OP_JMP_GT_IMPLICIT,
	OP_JMP_GT_CONST,
	OP_JMP_GT_REG,
	// ------------------------------------------------------------------	

	// Jump if less than 0
	OP_JMP_LT_IMPLICIT,
	OP_JMP_LT_CONST,
	OP_JMP_LT_REG,
	// ------------------------------------------------------------------	
	
	// Jump if greater or equal to 0
	OP_JMP_GE_IMPLICIT,
	OP_JMP_GE_CONST,
	OP_JMP_GE_REG,
	// ------------------------------------------------------------------	

	// Jump if less or equal to 0
	OP_JMP_LE_IMPLICIT,
	OP_JMP_LE_CONST,
	OP_JMP_LE_REG,
	// ------------------------------------------------------------------	


	// Add signed integers
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_SADD_IMPLICIT,
	OP_SADD_CONST,
	OP_SADD_REG,
	// ------------------------------------------------------------------	

	// Subtract signed integers
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_SSUB_IMPLICIT,
	OP_SSUB_CONST,
	OP_SSUB_REG,
	// ------------------------------------------------------------------	
	
	// Multiply signed integers
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_SMUL_IMPLICIT,
	OP_SMUL_CONST,
	OP_SMUL_REG,
	// ------------------------------------------------------------------	
	
	// Divide signed integers
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_SDIV_IMPLICIT,
	OP_SDIV_CONST,
	OP_SDIV_REG,
	// ------------------------------------------------------------------	

	// Add unsigned integers
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_UADD_IMPLICIT,
	OP_UADD_CONST,
	OP_UADD_REG,
	// ------------------------------------------------------------------	

	// Subtract unsigned integers
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_USUB_IMPLICIT,
	OP_USUB_CONST,
	OP_USUB_REG,
	// ------------------------------------------------------------------	
	
	// Multiply unsigned integers
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_UMUL_IMPLICIT,
	OP_UMUL_CONST,
	OP_UMUL_REG,
	// ------------------------------------------------------------------	
	
	// Divide unsigned integers
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_UDIV_IMPLICIT,
	OP_UDIV_CONST,
	OP_UDIV_REG,
	// ------------------------------------------------------------------	


	// Add float
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_ADD_IMPLICIT_UNUSED,
	OP_ADD_CONST,
	OP_ADD_REG,
	// ------------------------------------------------------------------	

	// Subtract float
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_SUB_IMPLICIT_UNUSED,
	OP_SUB_CONST,
	OP_SUB_REG,
	// ------------------------------------------------------------------	
	
	// Multiply float
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_MUL_IMPLICIT_UNUSED,
	OP_MUL_CONST,
	OP_MUL_REG,
	// ------------------------------------------------------------------	
	
	// Divide float
	// 1st source operand is always a register. 2nd operand depends on op code
	OP_DIV_IMPLICIT_UNUSED,
	OP_DIV_CONST,
	OP_DIV_REG,
	// ------------------------------------------------------------------	

	// Debug prints
	OP_DEBUG_STRING,
	OP_DEBUG_CONSTANT,
	OP_DEBUG_REG,

	OP_HALT, // Must be last element
} Op;

#define OPS_COUNT (OP_HALT + 1)
#define INST_SIZE sizeof(Inst)
#define MAX_INST_SIZE 1024

#define UINT16_ARG(arg1, arg2) ((((uint16_t)(arg2)) << 8LL) + (arg1))

static_assert(sizeof(Inst)==4, "Unexpected size of INST");

inline Inst create_inst0(Op type) {
	return (Inst) { .op=type, .args={0} };
}

inline Inst create_inst1(Op type, uint8_t a1) {
	return (Inst) { .op=type, .args={a1, 0, 0} };
}

inline Inst create_inst2(Op type, uint8_t a1, uint8_t a2) {
	return (Inst) { .op=type, .args={a1, a2, 0} };
}

inline Inst create_inst3(Op type, uint8_t a1, uint8_t a2, uint8_t a3) {
	return (Inst) { .op=type, .args={a1, a2, a3} };
}

void debug_inst(const Inst* inst, char* debugStr, size_t strLen);

#endif
