
#define SV_IMPLEMENTATION
#include <sv.h>

#include <stdlib.h>
#include "./bytecode_parser.c"


// --CODE DUPLICATION START--
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
// --CODE DUPLICATION END--

static void debug_write_program(const Program* program, const char* file_path) {
	FILE* file = fopen(file_path, "w");
	if (file == NULL) {
		fprintf(stderr, "Unable to open file %s, %s\n", file_path, strerror(errno));
		exit(1);
	}

	for (size_t i=0; i<vector_size(program->instructions); ++i) {
		char str[1024];
		debug_inst(&program->instructions[i], str, 1024);
		fprintf(file, "%s\n", str);
	}

	fprintf(file, "\n\n[Constants]\n");

	for (size_t i=0; i<program->header.constantsCount; ++i) {
		uint64_t contents = program->constants[i];
		fprintf(file, "unsigned: %lu, signed:%li, float: %lf\n", contents, *(int64_t*)&contents, *(double*)&contents);
	}

	fprintf(file, "\n\n[Strings]\n");
	for (size_t i=0; i<program->header.stringsCount; ++i) {
		String s = program->strings[i];
		fprintf(file, "%lu: %.*s\n", i, (int)s.len, s.str);
	}

	fclose(file);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s <file_path.vm> -o <output_file.dbg>\n", argv[0]);
		exit(1);
	}

	const char* input_file = argv[1];

	char* output_file = NULL;

	while (argc > 2) {
		if (strcmp(argv[2], "-o") == 0) {
			if (argc > 3) {
				output_file = argv[3];
			}
		}
	}
	if (!output_file) {
		output_file = get_file_with_extension(sv_from_cstr(input_file), sv_from_cstr("dbg"));
	}

	Program* program = load_program_from_file(input_file);
	debug_write_program(program, output_file);

	return 0;
}
