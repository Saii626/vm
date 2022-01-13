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
	if (argc < 2) {
		printf("Usage: %s <file_path.vm>\n", argv[0]);
		exit(1);
	}

	Program p = load_program_from_file(argv[1]);
	execute_program(&vm, p);

	return 0;
}


