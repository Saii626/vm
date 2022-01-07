
#include <inst.h>

Inst create_inst0(Op type) {
	return (Inst) { .op=type, .args={0} };
}

Inst create_inst1(Op type, uint8_t a1) {
	return (Inst) { .op=type, .args={a1, 0, 0} };
}

Inst create_inst2(Op type, uint8_t a1, uint8_t a2) {
	return (Inst) { .op=type, .args={a1, a2, 0} };
}

Inst create_inst3(Op type, uint8_t a1, uint8_t a2, uint8_t a3) {
	return (Inst) { .op=type, .args={a1, a2, a3} };
}

Inst parse_inst(const char* inst) {
	return (Inst) { .op=OP_NOOP };
}

const char* describe(Inst* inst) {
	switch (inst->op) {
		case OP_NOOP:			return "OP_NOOP";

		case OP_LOAD_CONST: 		return "OP_LOAD_CONST";
		case OP_LOAD_REG: 		return "OP_LOAD_REG";

		case OP_JMP_CONST: 		return "OP_JUMP_CONST";
		case OP_JMP_REG: 		return "OP_JUMP_REG";

		case OP_CMP_JMP_CONST: 		return "OP_COMPARE_JUMP_CONST";
		case OP_CMP_JMP: 		return "OP_COMPARE_JUMP_REG";

		case OP_EQ_CONST: 		return "OP_COMPARE_EQUAL_CONST";
		case OP_EQ_REG: 		return "OP_COMPARE_EQUAL_REG";

		case OP_ADD_CONST: 		return "OP_ADD_CONST";
		case OP_ADD_REG: 		return "OP_ADD_REG";

		case OP_DEBUG_PRINT: 		return "OP_DEBUG_PRINT";
		case OP_HALT: 			return "HALT";
		default: 			return "unknown instruction";
	} 
}
