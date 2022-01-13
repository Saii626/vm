
#define SV_IMPLEMENTATION
#include <compiler.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

extern Inst create_inst0(Op type);

static bool isNotBlank(char ch) {
	return !isblank(ch);
}

//static bool isBlank(char ch) {
//	return isblank(ch);
//}

// --CODE DUPLICATION START--
static bool isNotDot(char ch) {
	return ch != '.';
}
// --CODE DUPLICATION END--

static bool isNotQuote(char ch) {
	return ch != '"';
}

static Operand chop_operand(String_View* str, String_View line, uint64_t lineNo) {
	*str = sv_trim_left(*str);

	uint64_t index = str->data - line.data;
	if (str->count == 0) {
		return (Operand) { .type=NONE, { .integer=0 }, .lineNo=lineNo, .index=index };
	}
	else if (sv_starts_with(*str, sv_from_cstr(":"))) {
		sv_chop_left(str, 1);
		String_View name = sv_chop_left_while(str, isNotBlank); 					\
		return (Operand) { .type=LABEL, { .label=name }, .lineNo=lineNo, .index=index };
	}
	else if (sv_starts_with(*str, sv_from_cstr("\""))) {
		sv_chop_left(str, 1); // chop left "
		String_View string = sv_chop_left_while(str, isNotQuote);
		sv_chop_left(str, 1); // chop right "
		return (Operand) { .type=STR, { .string=string }, .lineNo=lineNo, .index=index };
	}
	else if (isalpha(*str->data)){
		String_View name = sv_chop_left_while(str, isNotBlank); 					\
		return (Operand) { .type=VAR, { .variable=name }, .lineNo=lineNo, .index=index };
	}
	else {
		String_View number = sv_chop_left_while(str, isNotBlank); 					\
		char* lastPos;
		uint64_t num = (uint64_t) strtol(number.data, &lastPos, 0);
		if (*lastPos != '.') {
			return (Operand) { .type=CONST_I, { .integer=num }, .lineNo=lineNo, .index=index };
		} else {
			double num1 = strtod(number.data, NULL);
			return (Operand) { .type=CONST_F, { .floating=num1 }, .lineNo=lineNo, .index=index };
		}
	}
}

#define TRY_COMPILE(op) \
	else if (sv_eq(baseInst, sv_from_cstr(#op))) {								\
		Operand op1 = chop_operand(&inst, line, context.lineNo); 	\
		Operand op2 = chop_operand(&inst, line, context.lineNo); 	\
		Operand op3 = chop_operand(&inst, line, context.lineNo); 	\
		compile_##op(&context, op1, op2, op3); 						\
	} 													\

static int resolve_labels(void* const context, struct hashmap_element_s* const e) {
	String_View key = sv_from_parts(e->key, e->key_len);

	Context* ctx = context;

	void* label = hashmap_get(&ctx->labels, key.data, key.count);
	if (label == NULL) {
		fprintf(stderr, "No '"SV_Fmt"' label found\n", SV_Arg(key));
		exit(1);
	} else {
		uint64_t index = (uint64_t) label;

		for (size_t i=0; i<vector_size(e->data); ++i) {
			uint8_t* toReplace = ((uint8_t**)e->data)[i];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
			uint64_t currIndex = ((void*)toReplace - (void*)(ctx->program->instructions)) / sizeof(Inst);
#pragma GCC diagnostic pop
			int64_t diff = index - currIndex;
			fprintf(stderr, "label: "SV_Fmt" index: %lu, currIndex: %lu\n", SV_Arg(key), index, currIndex);

			if (diff >= INT16_MIN && diff <=INT16_MAX) {
				uint16_t udiff = (uint16_t) diff;
				*toReplace = (uint8_t) udiff;
				*(toReplace + 1) = (uint8_t) (udiff >> 8);
			} else {
				fprintf(stderr, "Jump size %li too large to fit in 16 bytes\n", (int64_t)diff);
				exit(1);
			}
		}
	}

	return 0;
}

static uint8_t resolve_variable(String_View name, Context* context) {
	void* p = hashmap_get(&context->variables, name.data, name.count);
	if (p) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvoid-pointer-to-int-cast"
		return ((uint8_t) p) - 1;
#pragma GCC diagnostic pop
	} else {
		fprintf(stderr, ""CTX_DEBUG_FMT" No '"SV_Fmt"' variable declared\n", CTX_DEBUG(context), SV_Arg(name));
		exit(1);
	}
}

static uint16_t resolve_constant(int64_t value, Context* context, bool force1Byte) {
	char number[64];
	snprintf(number, sizeof(number), "%li", value);
	size_t size = strlen(number);

	void* p = hashmap_get(&context->constants, number, size);
	if (!p) {
		if (context->constants_index == UINT16_MAX) {
			fprintf(stderr, ""CTX_DEBUG_FMT" No more space for constants left\n", CTX_DEBUG(context));
			exit(1);
		}

		if (force1Byte && context->constants_index >= UINT8_MAX) {
			fprintf(stderr, ""CTX_DEBUG_FMT" Unable to resolve constant to 1 byte index\n", CTX_DEBUG(context));
			exit(1);
		}

		context->program->constants[context->constants_index] = value;
		uint16_t index = context->constants_index++;

		p = (void*)((uint64_t)index + 1);
		hashmap_put(&context->constants, number, size, p);
	}

	return (uint16_t)((uint64_t) p - 1);
}

static uint16_t resolve_string(String_View value, Context* context) {
	void* p = hashmap_get(&context->constants, value.data, value.count);
	if (!p) {
		if (context->strings_index == UINT16_MAX) {
			fprintf(stderr, ""CTX_DEBUG_FMT" No more space for strings left\n", CTX_DEBUG(context));
			exit(1);
		}

		char* str = malloc(sizeof(char) * value.count);
		memcpy(str, value.data, value.count);

		context->program->strings[context->strings_index] = (String) { .str=str, .len=value.count };
		uint16_t index = context->strings_index++;

		p = (void*)((uint64_t)index + 1);
		hashmap_put(&context->constants, value.data, value.count, p);
	}

	return (uint16_t)((uint64_t) p - 1);
}

void resolve_all_symbols(Context* context) {
	hashmap_iterate_pairs(&context->unresolvedLabels, resolve_labels, context);
}

static InstVariant resolve_one_byte_arg(Context* context, Operand operand, uint8_t* loc) {
	uint8_t val = 0;
	InstVariant variant = IMPLICIT;

	switch (operand.type) {
		case NONE:
			fprintf(stderr, ""CTX_DEBUG_FMT" Operand expected but not found\n", CTX_DEBUG(context));
			exit(1);
		case VAR:
			val = resolve_variable(operand.variable, context);
			variant = REG;
			break;
		case CONST_I: {
				int64_t v = operand.integer;
				if (v > UINT8_MAX || v < INT8_MIN) {
					val = (uint8_t)resolve_constant(v, context, true);
					variant = CONST;
				} else {
					val = (uint8_t) v;
					variant = IMPLICIT;
				}
				break;
			}
		case CONST_F: {
				double v = operand.floating;
				val = (uint8_t)resolve_constant(*(uint64_t*)&v, context, true);
				variant = CONST;
				break;
			}
		case LABEL:
			fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected label\n", CTX_DEBUG(context));
			exit(1);
		case STR:
			fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected String\n", CTX_DEBUG(context));
			exit(1);
		default:
			fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand\n", CTX_DEBUG(context));
			exit(1);
	}

	if (loc) {
		*loc = val;
	}

	return variant;
}

static InstVariant resolve_two_byte_arg(Context* context, Operand operand, uint8_t* loc) {
	uint16_t val = 0;
	InstVariant variant = IMPLICIT;

	switch (operand.type) {
		case NONE:
			fprintf(stderr, ""CTX_DEBUG_FMT" Operand expected but not found\n", CTX_DEBUG(context));
			exit(1);
		case VAR:
			val = resolve_variable(operand.variable, context);
			variant = REG;
			break;
		case CONST_I: {
				uint64_t v = operand.integer;
				if (v > UINT16_MAX) {
					val = resolve_constant(v, context, false);
					variant = CONST;
				} else {
					val = (uint16_t) v;
					variant = IMPLICIT;
				}
				break;
			}
		case CONST_F: {
				double v = operand.floating;
				val = resolve_constant(*(uint64_t*)&v, context, false);
				variant = CONST;
				break;
			}
		case STR: {
				val = resolve_string(operand.string, context);
				variant = STRING;
				break;
			}
		case LABEL:
			if (loc) {
				uint8_t** usages = hashmap_get(&context->unresolvedLabels, operand.label.data, operand.label.count);
				vector_push_back(usages, loc);
				hashmap_put(&context->unresolvedLabels, operand.label.data, operand.label.count, usages);
				val = 0;
				variant = IMPLICIT;
			} else {
				fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected label\n", CTX_DEBUG(context));
				exit(1);
			}
			break;
		default:
			fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand\n", CTX_DEBUG(context));
			exit(1);
	}

	if (loc) {
		*loc = (uint8_t)val;
		*(loc + 1) = (uint8_t) (val >> 8);
	}
	return variant;
}

static void assert_operand(Context* ctx, Operand op, OperandType type) {
	if (op.type != type) {
		fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand. expected %s, found %s\n", ctx->fileName, ctx->lineNo, op.index, describe_operand_type(type), describe_operand_type(op.type));
		exit(1);
	}
}

static Inst* insert_inst_at_end(Context* context, Inst inst) {
	Inst* inst_vector = context->program->instructions;
	vector_push_back(inst_vector, inst);
	context->program->instructions = inst_vector;
	return &inst_vector[vector_size(inst_vector)-1];
}

static void compile_noop(Context* context, Operand arg1, Operand arg2, Operand arg3) {
	assert_operand(context, arg1, NONE);
	assert_operand(context, arg2, NONE);
	assert_operand(context, arg3, NONE);
	insert_inst_at_end(context, create_inst0(OP_NOOP));
}

#define ARG_LOC(arg) ((uint8_t*)inst + (arg) + 1)

static void compile_load(Context* context, Operand arg1, Operand arg2, Operand arg3) {
	Inst* inst = insert_inst_at_end(context, create_inst0(OP_LOAD_IMPLICIT));

	assert_operand(context, arg1, VAR);
	assert_operand(context, arg3, NONE);

	resolve_one_byte_arg(context, arg1, ARG_LOC(0));
	InstVariant variant = resolve_two_byte_arg(context, arg2, ARG_LOC(1));
	inst->op += variant;
}

static void compile_jmp(Context* context, Operand arg1, Operand arg2, Operand arg3) {
	Inst* inst = insert_inst_at_end(context, create_inst0(OP_JMP_IMPLICIT));

	assert_operand(context, arg2, NONE);
	assert_operand(context, arg3, NONE);

	InstVariant variant = resolve_two_byte_arg(context, arg1, ARG_LOC(0));
	inst->op += variant;
}

#define COMPILE_CONDITIONAL_JMP(fn_name, op_name) 							\
static void compile_##fn_name(Context* context, Operand arg1, Operand arg2, Operand arg3) { 		\
	Inst* inst = insert_inst_at_end(context, create_inst0( (op_name) )); 				\
													\
	assert_operand(context, arg1, VAR); 								\
	assert_operand(context, arg3, NONE); 								\
													\
	resolve_one_byte_arg(context, arg1, ARG_LOC(0)); 						\
	InstVariant variant = resolve_two_byte_arg(context, arg2, ARG_LOC(1)); 				\
	inst->op += variant; 										\
} 													\

COMPILE_CONDITIONAL_JMP(jze, OP_JMP_ZE_IMPLICIT)
COMPILE_CONDITIONAL_JMP(jnz, OP_JMP_NZ_IMPLICIT)
COMPILE_CONDITIONAL_JMP(jgt, OP_JMP_GT_IMPLICIT)
COMPILE_CONDITIONAL_JMP(jlt, OP_JMP_LT_IMPLICIT)
COMPILE_CONDITIONAL_JMP(jge, OP_JMP_GE_IMPLICIT)
COMPILE_CONDITIONAL_JMP(jle, OP_JMP_LE_IMPLICIT)

#define COMPILE_BINARY_OPERATION(fn_name, op_name) 							\
static void compile_##fn_name(Context* context, Operand arg1, Operand arg2, Operand arg3) { 		\
	Inst* inst = insert_inst_at_end(context, create_inst0( (op_name) )); 				\
													\
	assert_operand(context, arg1, VAR); 								\
	assert_operand(context, arg2, VAR); 								\
													\
	resolve_one_byte_arg(context, arg1, ARG_LOC(0)); 						\
	resolve_one_byte_arg(context, arg2, ARG_LOC(1)); 						\
	InstVariant variant = resolve_one_byte_arg(context, arg3, ARG_LOC(2)); 				\
	inst->op += variant; 										\
} 													\

COMPILE_BINARY_OPERATION(sadd, OP_SADD_IMPLICIT)
COMPILE_BINARY_OPERATION(ssub, OP_SSUB_IMPLICIT)
COMPILE_BINARY_OPERATION(smul, OP_SMUL_IMPLICIT)
COMPILE_BINARY_OPERATION(sdiv, OP_SDIV_IMPLICIT)
COMPILE_BINARY_OPERATION(uadd, OP_UADD_IMPLICIT)
COMPILE_BINARY_OPERATION(usub, OP_USUB_IMPLICIT)
COMPILE_BINARY_OPERATION(umul, OP_UMUL_IMPLICIT)
COMPILE_BINARY_OPERATION(udiv, OP_UDIV_IMPLICIT)

COMPILE_BINARY_OPERATION(add,  OP_ADD_IMPLICIT_UNUSED)
COMPILE_BINARY_OPERATION(sub,  OP_SUB_IMPLICIT_UNUSED)
COMPILE_BINARY_OPERATION(mul,  OP_MUL_IMPLICIT_UNUSED)
COMPILE_BINARY_OPERATION(div,  OP_DIV_IMPLICIT_UNUSED)

#define COMPILE_DEBUG_PRINT(fn_name, op_name) 								\
static void compile_##fn_name(Context* context, Operand arg1, Operand arg2, Operand arg3) { 			\
	assert_operand(context, arg1, VAR); 								\
	assert_operand(context, arg2, NONE); 								\
	assert_operand(context, arg3, NONE); 								\
													\
	Inst* inst = insert_inst_at_end(context, create_inst0( (op_name) )); 				\
	resolve_two_byte_arg(context, arg1, ARG_LOC(0)); 						\
} 													\

COMPILE_DEBUG_PRINT(print_reg, OP_DEBUG_REG)
COMPILE_DEBUG_PRINT(print_const, OP_DEBUG_CONSTANT)
COMPILE_DEBUG_PRINT(print_string, OP_DEBUG_STRING)

static void compile_halt(Context* context, Operand arg1, Operand arg2, Operand arg3) {
	assert_operand(context, arg1, NONE);
	assert_operand(context, arg2, NONE);
	assert_operand(context, arg3, NONE);

	insert_inst_at_end(context, create_inst0(OP_HALT));
}

const char* describe_operand_type(OperandType type) {
	switch (type) {
		case NONE: return "None";
		case VAR: return "Variable";
		case CONST_I: return "Integer";
		case CONST_F: return "Floating";
		case LABEL: return "Label";
		case STR: return "String";
		default: return "Unknown operand";
	}
}

void finaize_program(Context* context) {
	resolve_all_symbols(context);

	Program* program = context->program;
	program->header.instructionsCount = vector_size(program->instructions);
	program->header.constantsCount = context->constants_index;
	program->header.stringsCount = context->strings_index;

	uint64_t totalStringSize = 0;
	for (size_t i=0; i<context->strings_index; i++) {
		String s = program->strings[i];
		totalStringSize += s.len;
	}

	program->header.stringsSize = totalStringSize;
}

void write_prog_to_file(const char* file_path, const Program* program) {
	{
		FILE* file = fopen(file_path, "wb");
		if (file == NULL) {
			fprintf(stderr, "Unable to open file %s, %s\n", file_path, strerror(errno));
			exit(1);
		}

		if (fwrite(&program->header, sizeof(ProgramHeader), 1, file) != 1) {
			fprintf(stderr, "Unable to write ProgramHeader to file %s, %s\n", file_path, strerror(errno));
			exit(1);
		}

		size_t noOfInst = program->header.instructionsCount;
		size_t itemsWrote = fwrite(program->instructions, sizeof(Inst), noOfInst, file);
		if (itemsWrote != noOfInst) {
			fprintf(stderr, "Unable to write Instructions to file %s, %s\n", file_path, strerror(errno));
			exit(1);
		}

		size_t noOfConst = program->header.constantsCount;
		itemsWrote = fwrite(program->constants, sizeof(uint64_t), noOfConst, file);
		if (itemsWrote != noOfConst) {
			fprintf(stderr, "Unable to write Constant to file %s, %s\n", file_path, strerror(errno));
			exit(1);
		}

		size_t noOfString = program->header.stringsCount;
		for (size_t i=0; i<noOfString; i++) {
			String s = program->strings[i];
			if (fwrite(&s.len, sizeof(uint64_t), 1, file) != 1) {
				fprintf(stderr, "Unable to write String to file %s, %s\n", file_path, strerror(errno));
				exit(1);
			}

			itemsWrote = fwrite(s.str, sizeof(char), s.len, file);
			if (itemsWrote != s.len) {
				fprintf(stderr, "Unable to write String to file %s, %s\n", file_path, strerror(errno));
				exit(1);
			}
		}

		fclose(file);
	}
}


Program* compile_file(const char* file_path) {
	// Open the input file and read its contents
	char* file_contents;
	{
		FILE* file = fopen(file_path, "r");
		if (file == NULL) {
			fprintf(stderr, "Unable to open file %s, %s\n", file_path, strerror(errno));
			exit(1);
		}

		if (fseek(file, 0L, SEEK_END) != 0) {
			fprintf(stderr, "Unable to seek to end of file %s, %s\n", file_path, strerror(errno));
			fclose(file);
			exit(1);
		};

		long size = ftell(file);
		if (size == -1) {
			fprintf(stderr, "Unable to get size of file %s, %s\n", file_path, strerror(errno));
			fclose(file);
			exit(1);
		};

		if (fseek(file, 0L, SEEK_SET) != 0) {
			fprintf(stderr, "Unable to seek to start of file %s, %s\n", file_path, strerror(errno));
			fclose(file);
			exit(1);
		};

		file_contents = malloc(size * sizeof(char));
		size_t n = fread(file_contents, 1, size * sizeof(char), file);
		if (n != (size_t)size) {
			fprintf(stderr, "Unable to read file %s, %s\n", file_path, strerror(errno));
			fclose(file);
			exit(1);
		}
		fclose(file);
	}

	// Create and initialize a new Context for compilation
	Context context = {0};
	context.fileName = file_path;

	Program* program = malloc(sizeof(Program));
	context.program = program;
	{
		if (0 != hashmap_create(pow(2, 10), &context.labels)) {
			fprintf(stderr, "Unable to create label hashmap\n");
			exit(1);
		}

		if (0 != hashmap_create(pow(2, 10), &context.variables)) {
			fprintf(stderr, "Unable to create variables hashmap\n");
			exit(1);
		}

		if (0 != hashmap_create(pow(2, 10), &context.unresolvedLabels)) {
			fprintf(stderr, "Unable to create unresolved hashmap\n");
			exit(1);
		}
		if (0 != hashmap_create(pow(2, 10), &context.constants)) {
			fprintf(stderr, "Unable to create constants hashmap\n");
			exit(1);
		}
		if (0 != hashmap_create(pow(2, 10), &context.strings)) {
			fprintf(stderr, "Unable to create strings hashmap\n");
			exit(1);
		}
	}

	// Compile the program
	{
		String_View contents = sv_from_cstr(file_contents);
		
		while (contents.count > 0) {
			String_View line = sv_chop_by_delim(&contents, '\n');
			context.lineNo += 1;

			String_View trimmed_line = sv_trim(line);
			if (trimmed_line.count > 0 && *(trimmed_line.data) != '#') {

				// Remove comments at the end of an instruction
				String_View inst = sv_trim(sv_chop_by_delim(&trimmed_line, '#'));

				if (inst.count > 0) {
					context.index = (inst.data - line.data);
					String_View baseInst = sv_chop_left_while(&inst, isNotBlank);

					if (sv_eq(baseInst, sv_from_cstr("var"))) {
						String_View varName = sv_trim(inst);
						context.index = (varName.data - line.data);

						if (hashmap_get(&context.variables, varName.data, varName.count) != NULL) {
							fprintf(stderr, ""CTX_DEBUG_FMT" Variable '"SV_Fmt"' redefined\n", CTX_DEBUG(&context), SV_Arg(varName));
							exit(1);
						}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-void-pointer-cast"
						hashmap_put(&context.variables, varName.data, varName.count, (void*)++context.reg_index);
#pragma GCC diagnostic pop
						if (context.reg_index == UINT8_MAX) {
							fprintf(stderr, ""CTX_DEBUG_FMT" Register capacity reached. Unable to declare anymore variables\n", CTX_DEBUG(&context));
							exit(1);
						}
					} else if (sv_ends_with(baseInst, sv_from_cstr(":"))){
						sv_chop_right(&baseInst, 1);

						if (hashmap_get(&context.labels, baseInst.data, baseInst.count) != NULL) {
							fprintf(stderr, ""CTX_DEBUG_FMT" Label '"SV_Fmt"' redefined\n", CTX_DEBUG(&context), SV_Arg(baseInst));
							exit(1);
						}
						hashmap_put(&context.labels, baseInst.data, baseInst.count, (void*)(vector_size(context.program->instructions)));
					}
					TRY_COMPILE(noop)

					TRY_COMPILE(load)

					TRY_COMPILE(jmp)
					TRY_COMPILE(jze)
					TRY_COMPILE(jnz)
					TRY_COMPILE(jgt)
					TRY_COMPILE(jlt)
					TRY_COMPILE(jge)
					TRY_COMPILE(jle)

					TRY_COMPILE(sadd)
					TRY_COMPILE(ssub)
					TRY_COMPILE(smul)
					TRY_COMPILE(sdiv)
					TRY_COMPILE(uadd)
					TRY_COMPILE(usub)
					TRY_COMPILE(umul)
					TRY_COMPILE(udiv)

					TRY_COMPILE(add)
					TRY_COMPILE(sub)
					TRY_COMPILE(mul)
					TRY_COMPILE(div)

					TRY_COMPILE(print_reg)
					TRY_COMPILE(print_const)
					TRY_COMPILE(print_string)

					TRY_COMPILE(halt)
					else {
						fprintf(stderr, ""CTX_DEBUG_FMT" Unknown instruction '"SV_Fmt"'\n", CTX_DEBUG(&context), SV_Arg(baseInst));
						exit(1);
					}
				}
			}
		}

	}

	// Resolve any remaining symbols and generate program headers
	finaize_program(&context);

	// Generate program headers
	return context.program;
}



// --CODE DUPLICATION START--
static char* get_file_with_extension(String_View file_path, String_View ext) {
	sv_chop_right_while(&file_path, isNotDot);
	char* newName = malloc(sizeof(char) * (file_path.count + ext.count + 1));
	memcpy(newName, file_path.data, file_path.count);
	memcpy(newName + (file_path.count), ext.data, ext.count);
	newName[file_path.count + ext.count] = '\0';
	return newName;
}
// --CODE DUPLICATION END--

int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("Usage: %s <file_path.src> -o <output_file.vm>\n", argv[0]);
		exit(1);
	}

	int index = 2;
	const char* input_file = argv[1];

	char* output_file = NULL;

	while (argc > index) {
		if (strcmp(argv[index], "-o") == 0) {
			index += 1;
			if (argc > index) {
				output_file = argv[index];
				index += 1;
			}
		}
	}
	if (!output_file) {
		output_file = get_file_with_extension(sv_from_cstr(input_file), sv_from_cstr("vm"));
	}
	
	Program* program = compile_file(input_file);
	write_prog_to_file(output_file, program);
}
