
#ifndef __BYTECODE_PARSER_H__
#define __BYTECODE_PARSER_H__

#include <inst.h>
#include <vector.h>
#include <errno.h>

Program* load_program_from_file(const char* file_path) {

	FILE* file = fopen(file_path, "rb");
	if (file == NULL) {
		fprintf(stderr, "Unable to open file %s, %s\n", file_path, strerror(errno));
		exit(1);
	}

	Program* program = malloc(sizeof(Program));

	if (fread(&program->header, sizeof(ProgramHeader), 1, file) != 1) {
		fprintf(stderr, "Unable to read ProgramHeader from file %s, %s\n", file_path, strerror(errno));
		exit(1);
	}

	uint64_t noOfInst = program->header.instructionsCount;
	vector_grow(program->instructions, noOfInst);
	if (fread(program->instructions, sizeof(Inst), noOfInst, file) != noOfInst) {
		fprintf(stderr, "Unable to read Instructions from file %s, %s\n", file_path, strerror(errno));
		exit(1);
	}
	vector_set_size(program->instructions, noOfInst);

	uint64_t noOfConst = program->header.constantsCount;
	if (fread(program->constants, sizeof(uint64_t), noOfConst, file) != noOfConst) {
		fprintf(stderr, "Unable to read Constant from file %s, %s\n", file_path, strerror(errno));
		exit(1);
	}

	uint64_t noOfString = program->header.stringsCount;
	uint64_t stringSize = program->header.stringsSize;
	char* staticStrings = malloc(stringSize * sizeof(char));

	for (size_t i=0; i<noOfString; i++) {
		uint64_t stringSize;
		if (fread(&stringSize, sizeof(uint64_t), 1, file) != 1) {
			fprintf(stderr, "Unable to read String size from file %s, %s\n", file_path, strerror(errno));
			exit(1);
		}

		if (fread(staticStrings, sizeof(char), stringSize, file) != stringSize) {
			fprintf(stderr, "Unable to read String from file %s, %s\n", file_path, strerror(errno));
			exit(1);
		}

		program->strings[i] = (String) { .str=staticStrings, .len=stringSize };
		staticStrings += stringSize;
	}

	fclose(file);

	return program;
}

#endif
