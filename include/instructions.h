
#include <stdint.h>

typedef struct {
	uint8_t op;
	uint8_t args[3];
} Inst;

typedef enum {
	OP_NOOP = 0,

	OP_LOAD_CONST,
	OP_LOAD_REG,

	OP_JMP_CONST,
	OP_JMP_REG,

	OP_CMP_JMP_CONST,
	OP_CMP_JMP,

	OP_EQ_CONST,
	OP_EQ_REG,

	OP_ADD_CONST,
	OP_ADD_REG,

	OP_DEBUG_PRINT,
	OP_HALT, // Must be last element
} Op;

#define OPS_COUNT (OP_HALT + 1)

Inst create_inst0(Op instruction_type);
Inst create_inst1(Op instruction_type, uint8_t a1);
Inst create_inst2(Op instruction_type, uint8_t a1, uint8_t a2);
Inst create_inst3(Op instruction_type, uint8_t a1, uint8_t a2, uint8_t a3);

Inst parse_inst(const char* instruction);

const char* describe(Inst* inst);
