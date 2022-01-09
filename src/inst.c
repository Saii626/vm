
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

const char* describe(const Inst* inst) {
	switch (inst->op) {
		case OP_NOOP:			return "OP_NOOP";

		case OP_LOAD_CONST: 		return "OP_LOAD_CONST";
		case OP_LOAD_CONST2: 		return "OP_LOAD_CONST2";
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

void debug_inst(const Inst* inst, char* debugStr, size_t strLen) {
	switch (inst->op) {
		case OP_NOOP:
			snprintf(debugStr, strLen, "OP_NOOP");
			break;
		case OP_LOAD_CONST:
			snprintf(debugStr, strLen, "OP_LOAD_CONST %u %u", inst->args[0], inst->args[1]);
			break;
		case OP_LOAD_CONST2: {
			uint16_t num = ((uint16_t)inst->args[1]) << 8;
			snprintf(debugStr, strLen, "OP_LOAD_CONST2 %u %u", inst->args[0], (num + inst->args[2]));
			break;
		     }
		case OP_LOAD_REG:;
			snprintf(debugStr, strLen, "OP_LOAD_REG %u %u", inst->args[0], inst->args[1]);
			break;
		case OP_JMP_CONST: 
			snprintf(debugStr, strLen, "OP_JMP_CONST %i", *((int8_t*)&inst->args[0]));
			break;
		case OP_JMP_REG:
			snprintf(debugStr, strLen, "OP_JMP_REG %u", inst->args[0]);
			break;
		case OP_CMP_JMP_CONST:
			snprintf(debugStr, strLen, "OP_CMP_JMP_CONST %u %i %i", inst->args[0], *((int8_t*)&inst->args[1]), *((int8_t*)&inst->args[2]));
			break;
		case OP_CMP_JMP:
			snprintf(debugStr, strLen, "OP_CMP_JMP_REG %u %u %u", inst->args[0], inst->args[1], inst->args[2]);
			break;
		case OP_EQ_CONST:
			snprintf(debugStr, strLen, "OP_EQ_CONST %u %u %u", inst->args[0], inst->args[1], inst->args[2]);
			break;
		case OP_EQ_REG:
			snprintf(debugStr, strLen, "OP_EQ_REG %u %u %u", inst->args[0], inst->args[1], inst->args[2]);
			break;
		case OP_ADD_CONST:
			snprintf(debugStr, strLen, "OP_ADD_CONST %u %u %u", inst->args[0], inst->args[1], inst->args[2]);
			break;
		case OP_ADD_REG:
			snprintf(debugStr, strLen, "OP_ADD_REG %u %u %u", inst->args[0], inst->args[1], inst->args[2]);
			break;
		case OP_DEBUG_PRINT:
			snprintf(debugStr, strLen, "OP_DEBUG_PRINT %u", inst->args[0]);
			break;
		case OP_HALT:;
			snprintf(debugStr, strLen, "OP_HALT");
			break;
		default:
			snprintf(debugStr, strLen, "Unknown instruction");
			break;
	}
}
