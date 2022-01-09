#define VECTOR_LOGARITHMIC_GROWTH
#include <vector.h>
#include <stdio.h>
#include <vm.h>
#include <assert.h>
#include <errno.h>
#include <executors.h>

VM vm;

void execute_program(VM* vm, Program program) {
	vm->program = program;
	while (!vm->isHalted) {
		Inst inst = vm->program[vm->ip];

		(*executors[inst.op])(vm, inst.args);

		assert(vm->ip < vector_size(vm->program));
	}
}


Program load_program_from_file(const char* file_path) {
	FILE* file = fopen(file_path, "rb");
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

	Program program = NULL;
	uint64_t noOfInstructions = size / sizeof(Inst);
	vector_grow(program, noOfInstructions);

	size_t n = fread(program, sizeof(Inst), noOfInstructions, file);
	if (n != (size_t)noOfInstructions) {
		fprintf(stderr, "Unable to read file %s, %s\n", file_path, strerror(errno));
		fclose(file);
		exit(1);
	}

	vector_set_size(program, noOfInstructions);
	fclose(file);
	return program;
}

int main(int argc, char* argv[]) {
	//Program fib = NULL;
	//vector_push_back(fib, create_inst2(OP_LOAD_CONST, 0, 0)); 			// load 0 at `0
	//vector_push_back(fib, create_inst2(OP_LOAD_CONST, 1, 0)); 			// load 0 at `1
	//vector_push_back(fib, create_inst2(OP_LOAD_CONST, 2, 1)); 			// load 1 at `2
	//vector_push_back(fib, create_inst1(OP_DEBUG_PRINT, 1)); 			// print contents of `1
	//vector_push_back(fib, create_inst1(OP_DEBUG_PRINT, 2)); 			// print contents of `2

	//vector_push_back(fib, create_inst3(OP_ADD_REG, 3, 1, 2));     			// add num from `1 and `2 and store at `3
	//vector_push_back(fib, create_inst2(OP_LOAD_REG, 1, 2));   			// load `1 with `2
	//vector_push_back(fib, create_inst2(OP_LOAD_REG, 2, 3));   			// load `2 with `3
	//vector_push_back(fib, create_inst1(OP_DEBUG_PRINT, 3)); 			// print contents of `3

	//vector_push_back(fib, create_inst3(OP_ADD_CONST, 0, 0, 1));			// add 1 to `0 and store at `0
	//vector_push_back(fib, create_inst3(OP_EQ_CONST, 4, 0, 15)); 			// check if `0 == 15 and store in `4
	//vector_push_back(fib, create_inst3(OP_CMP_JMP_CONST, 4, 1, -6)); 		// if `4 is true, move to next line else go back 6 lines
	//vector_push_back(fib, create_inst0(OP_HALT));

	//load_program_in_vm(&vm, fib);
	//execute_program(&vm);
	if (argc < 2) {
		printf("Usage: %s <file_path.vm>\n", argv[0]);
		exit(1);
	}

	Program p = load_program_from_file(argv[1]);
	execute_program(&vm, p);

	return 0;
}


