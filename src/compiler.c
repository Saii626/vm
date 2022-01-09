
#define SV_IMPLEMENTATION
#include <compiler.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static Operand parse_operand(String_View str, String_View line, uint64_t lineNo) {
	String_View trimmed = sv_trim(str);

	uint64_t index = trimmed.data - line.data;
	if (trimmed.count == 0) {
		return (Operand) { .type=NONE, { .constant=0 }, .lineNo=lineNo, .index=index };
	} else if (sv_starts_with(trimmed, sv_from_cstr(":"))) {
		sv_chop_left(&trimmed, 1);
		return (Operand) { .type=LABEL, { .label=trimmed }, .lineNo=lineNo, .index=index };
	} else if (isdigit(*trimmed.data) || (trimmed.count>1 && *trimmed.data == '-' && isdigit(*(trimmed.data+1)))) {
		// TODO: Handle errors during parsing
		return (Operand) { .type=CONST, { .constant=atol(trimmed.data) }, .lineNo=lineNo, .index=index };
	} else {
		return (Operand) { .type=VAR, { .variable=trimmed }, .lineNo=lineNo, .index=index };
	}
}

#define TRY_COMPILE(op) \
	else if (sv_eq(baseInst, sv_from_cstr(#op))) {								\
		sv_chop_left_while(&inst, isBlank); 							\
		String_View i1 = sv_chop_left_while(&inst, isNotBlank); 					\
									\
		sv_chop_left_while(&inst, isBlank); 							\
		String_View i2 = sv_chop_left_while(&inst, isNotBlank); 					\
									\
		sv_chop_left_while(&inst, isBlank); 							\
		String_View i3 = sv_chop_left_while(&inst, isNotBlank); 					\
														\
		Operand op1 = parse_operand(i1, line, context.lineNo); 	\
		Operand op2 = parse_operand(i2, line, context.lineNo); 	\
		Operand op3 = parse_operand(i3, line, context.lineNo); 	\
		compile_##op(&program, &context, op1, op2, op3); 						\
	} 													\

static bool isNotBlank(char ch) {
	return !isblank(ch);
}

static bool isBlank(char ch) {
	return isblank(ch);
}


Program compile_file(const char* file_path) {
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

	}

	// Compile the program
	Program program = NULL;
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
						hashmap_put(&context.variables, varName.data, varName.count, (void*)++context.reg_index);
						if (context.reg_index == UINT8_MAX) {
							fprintf(stderr, ""CTX_DEBUG_FMT" Register capacity reached. Unable to declare anymore variables\n", CTX_DEBUG(&context));
							exit(1);
						}
					} else if (sv_ends_with(baseInst, sv_from_cstr(":"))){ // after chopping the alphaneumeric part, only : is left in the inst
						sv_chop_right(&baseInst, 1);

						if (hashmap_get(&context.labels, baseInst.data, baseInst.count) != NULL) {
							fprintf(stderr, ""CTX_DEBUG_FMT" Label '"SV_Fmt"' redefined\n", CTX_DEBUG(&context), SV_Arg(baseInst));
							exit(1);
						}
						hashmap_put(&context.labels, baseInst.data, baseInst.count, (void*)(vector_size(program)));
					}
					TRY_COMPILE(noop)
					TRY_COMPILE(load)
					TRY_COMPILE(jmp)
					TRY_COMPILE(cmp_jmp)
					TRY_COMPILE(eq)
					TRY_COMPILE(add)
					TRY_COMPILE(add)
					TRY_COMPILE(debug_print)
					TRY_COMPILE(halt)
					else {
						fprintf(stderr, ""CTX_DEBUG_FMT" Unknown instruction '"SV_Fmt"'\n", CTX_DEBUG(&context), SV_Arg(baseInst));
						exit(1);
					}
				}
			}
		}

	}

	// Resolve any remaining symbols
	resolve_all_symbols(&context, &program);

	return program;
}

void process_unresolved_label(Context* context, String_View name, uint8_t* loc) {
	uint8_t** val = hashmap_get(&context->unresolvedLabels, name.data, name.count);
	vector_push_back(val, loc);
	hashmap_put(&context->unresolvedLabels, name.data, name.count, val);
}


typedef struct {
	Context* ctx;
	Program* prog;
} IteratorContext;

static int iterate_over_unresolved_labels(void* const context, struct hashmap_element_s* const e) {
	String_View key = sv_from_parts(e->key, e->key_len);
	uint8_t** data = e->data;

	IteratorContext* itCtx = context;

	void* label = hashmap_get(&itCtx->ctx->labels, key.data, key.count);
	if (label == NULL) {
		fprintf(stderr, "No '"SV_Fmt"' label found\n", SV_Arg(key));
		exit(1);
	} else {
		uint64_t index = (uint64_t) label;

		uint8_t** it;
		for (it=vector_begin(data); it!=vector_end(data); ++it) {
			uint8_t* toReplace = *it;

			uint64_t currIndex = ((void*)toReplace - (void*)*(itCtx->prog)) / sizeof(Inst);
			int64_t diff = index - currIndex;

			if (diff >= INT8_MIN && diff <=INT8_MAX) {
				*toReplace = (uint8_t) diff;
			} else {
				fprintf(stderr, "Too big jump. Not supported\n");
				exit(1);
			}
		}
	}

	return 0;
}

void resolve_all_symbols(Context* context, Program* program) {
	IteratorContext ctx = { .ctx=context, .prog=program };
	hashmap_iterate_pairs(&context->unresolvedLabels, iterate_over_unresolved_labels, &ctx);
}


static uint8_t get_register_for_variable(String_View name, Context* context) {
	void* p = hashmap_get(&context->variables, name.data, name.count);
	if (p) {
		return ((uint8_t) p) - 1;
	} else {
		fprintf(stderr, ""CTX_DEBUG_FMT" No '"SV_Fmt"' variable declared\n", CTX_DEBUG(context), SV_Arg(name));
		exit(1);
	}
}

static uint8_t get_operand(Context* context, Operand operand, uint8_t* loc) {
	uint8_t val = 0;
	switch (operand.type) {
		case NONE:
			fprintf(stderr, ""CTX_DEBUG_FMT" Operand expected but not found\n", CTX_DEBUG(context));
			exit(1);
		case VAR:
			val = get_register_for_variable(operand.variable, context);
			break;
		case CONST:
			val = operand.constant;
			break;
		case LABEL:
			if (loc) {
				process_unresolved_label(context, operand.label, loc);
				val = 0;
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
		*loc = val;
	}
	return val;
}

static void assert_operand(Context* ctx, Operand op, OperandType type) {
	if (op.type != type) {
		fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand. expected %s, found %s\n", ctx->fileName, ctx->lineNo, op.index, describe_operand_type(type), describe_operand_type(op.type));
		exit(1);
	}
}

void compile_noop(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3) {
	Inst i = (Inst) {0};
	assert_operand(context, arg1, NONE);
	assert_operand(context, arg2, NONE);
	assert_operand(context, arg3, NONE);
	vector_push_back(*program, i);
}

void compile_load(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3) {
	Op op;
	assert_operand(context, arg1, VAR);
	assert_operand(context, arg3, NONE);

	if (arg2.type == CONST) {
		op = OP_LOAD_CONST;
	} else if (arg2.type == VAR) {
		op = OP_LOAD_REG;
	} else {
		fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand for load\n", CTX_DEBUG(context));
		exit(1);
	}
	uint8_t dest = get_operand(context, arg1, NULL);
	uint8_t src = get_operand(context, arg2, NULL);
	Inst i = (Inst) { .op=op, .args={dest, src, 0} };
	vector_push_back(*program, i);
}

void compile_jmp(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3) {
	Op op;
	assert_operand(context, arg2, NONE);
	assert_operand(context, arg3, NONE);

	if (arg1.type == CONST || arg1.type == LABEL) {
		op = OP_JMP_CONST;
	} else if (arg1.type == VAR) {
		op = OP_JMP_REG;
	} else {
		fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand for jmp\n", CTX_DEBUG(context));
		exit(1);
	}

	Inst i = (Inst) { .op=op, .args={0} };
	vector_push_back(*program, i);
	get_operand(context, arg1, &((*program)[vector_size(*program) - 1].args[0]));
}

void compile_cmp_jmp(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3) {
	Op op;
	assert_operand(context, arg1, VAR);

	if (arg2.type == CONST || arg2.type == LABEL) {
		op = OP_CMP_JMP_CONST;
		if (!(arg3.type == CONST || arg3.type == LABEL)) {
			fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand for cmp_jmp. Expected Constant or Label, found %s\n", CTX_DEBUG(context), describe_operand_type(arg3.type));
			exit(1);
		}
	} else if (arg2.type == VAR) {
		op = OP_CMP_JMP;
		assert_operand(context, arg3, VAR);
	} else {
		fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand for cmp_jmp\n", CTX_DEBUG(context));
		exit(1);
	}

	uint8_t dest = get_operand(context, arg1, NULL);
	Inst i = (Inst) { .op=op, .args={dest, 0, 0} };
	vector_push_back(*program, i);
	get_operand(context, arg2, &((*program)[vector_size(*program) - 1].args[1]));
	get_operand(context, arg3, &((*program)[vector_size(*program) - 1].args[2]));

}

void compile_eq(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3) {
	Op op;
	assert_operand(context, arg1, VAR);
	assert_operand(context, arg2, VAR);

	if (arg3.type == CONST) {
		op = OP_EQ_CONST;
	} else if (arg3.type == VAR) {
		op = OP_EQ_REG;
	} else {
		fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand for eq\n", CTX_DEBUG(context));
		exit(1);
	}
	uint8_t dest = get_operand(context, arg1, NULL);
	uint8_t lhs = get_operand(context, arg2, NULL);
	uint8_t rhs = get_operand(context, arg3, NULL);
	Inst i = (Inst) { .op=op, .args={dest, lhs, rhs} };
	vector_push_back(*program, i);
}

void compile_add(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3) {
	Op op;
	assert_operand(context, arg1, VAR);
	assert_operand(context, arg2, VAR);

	if (arg3.type == CONST) {
		op = OP_ADD_CONST;
	} else if (arg3.type == VAR) {
		op = OP_ADD_REG;
	} else {
		fprintf(stderr, ""CTX_DEBUG_FMT" Unexpected operand for add\n", CTX_DEBUG(context));
		exit(1);
	}
	uint8_t dest = get_operand(context, arg1, NULL);
	uint8_t lhs = get_operand(context, arg2, NULL);
	uint8_t rhs = get_operand(context, arg3, NULL);
	Inst i = (Inst) { .op=op, .args={dest, lhs, rhs} };
	vector_push_back(*program, i);
}

void compile_debug_print(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3) {
	assert_operand(context, arg1, VAR);
	assert_operand(context, arg2, NONE);
	assert_operand(context, arg3, NONE);

	uint8_t src = get_operand(context, arg1, NULL);
	Inst i = (Inst) { .op=OP_DEBUG_PRINT, .args={src, 0, 0}};
	vector_push_back(*program, i);
}

void compile_halt(Program* program, Context* context, Operand arg1, Operand arg2, Operand arg3) {
	assert_operand(context, arg1, NONE);
	assert_operand(context, arg2, NONE);
	assert_operand(context, arg3, NONE);

	Inst i = (Inst) { .op=OP_HALT, .args={0}};
	vector_push_back(*program, i);
}

const char* describe_operand_type(OperandType type) {
	switch (type) {
		case NONE: return "None";
		case VAR: return "Variable";
		case CONST: return "Constant";
		case LABEL: return "Label";
		default: return "Unknown operand";
	}
}

void write_prog_to_file(const char* file_path, const char* disasm_file_path, const Program* program) {
	{
		FILE* file = fopen(file_path, "wb");
		if (file == NULL) {
			fprintf(stderr, "Unable to open file %s, %s\n", file_path, strerror(errno));
			exit(1);
		}

		size_t noOfInst = vector_size(*program);
		size_t itemsWrote = fwrite(*program, sizeof(Inst), noOfInst, file);
		if (itemsWrote != noOfInst) {
			fprintf(stderr, "Unable to write to file %s, %s\n", file_path, strerror(errno));
			exit(1);
		}

		fclose(file);
	}

	if (disasm_file_path) {
		FILE* file = fopen(disasm_file_path, "w");
		if (file == NULL) {
			fprintf(stderr, "Unable to open file %s, %s\n", disasm_file_path, strerror(errno));
			exit(1);
		}

		size_t noOfInst = vector_size(*program);

		char instStr[MAX_INST_SIZE];
		for (size_t i=0; i<noOfInst; ++i) {
			debug_inst(&(*program)[i], instStr, MAX_INST_SIZE);
			size_t len = strlen(instStr);
			size_t bytesWrote = fwrite(instStr, sizeof(char), len, file);
			if (bytesWrote != len) {
				fprintf(stderr, "Unable to write to file %s, %s\n", file_path, strerror(errno));
				exit(1);
			}
			fwrite("\n", sizeof(char), 1, file);
		}

		fclose(file);

	}
}

static bool isNotDot(char ch) {
	return ch != '.';
}

static char* get_file_with_extension(String_View file_path, String_View ext) {
	sv_chop_right_while(&file_path, isNotDot);
	char* newName = malloc(sizeof(char) * (file_path.count + ext.count + 1));
	memcpy(newName, file_path.data, file_path.count);
	memcpy(newName + (file_path.count), ext.data, ext.count);
	newName[file_path.count + ext.count] = '\0';
	return newName;
}

int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("Usage: %s <file_path.src> -o <output_file.vm> -d <disassembled.dbg>\n", argv[0]);
		exit(1);
	}

	int index = 2;
	const char* input_file = argv[1];

	char* output_file = NULL;
	char* disassemblyFile = NULL;

	while (argc > index) {
		if (strcmp(argv[index], "-d") == 0) {
			index += 1;
			if (argc > index) {
				disassemblyFile = argv[index];
				index += 1;
			} else {
				disassemblyFile = get_file_with_extension(sv_from_cstr(input_file), sv_from_cstr("dbg"));
			}
		} else if (strcmp(argv[index], "-o") == 0) {
			index += 1;
			if (argc > index) {
				output_file = argv[index];
				index += 1;
			} else {
				output_file = get_file_with_extension(sv_from_cstr(input_file), sv_from_cstr("vm"));
			}
		}
	} 
	if (!output_file) {
		output_file = get_file_with_extension(sv_from_cstr(input_file), sv_from_cstr("vm"));
	}
	
	Program program = compile_file(input_file);
	write_prog_to_file(output_file, disassemblyFile, &program);
}
