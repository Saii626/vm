
#define SV_IMPLEMENTATION
#include <compiler.h>
#include <errno.h>
#include <stdio.h>

Inst* compile_file(const char* file_path) {
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

	uint64_t size = ftell(file);
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

	char* file_contents = malloc(size * sizeof(char));
	size_t n = fread(file_contents, 1, size * sizeof(char), file);
	if (n != size) {
		fprintf(stderr, "Unable to read file %s, %s\n", file_path, strerror(errno));
		fprintf(stderr, "Read %lu bytes, expected %lu bytes\n", n, size);
		fclose(file);
		exit(1);
	}
	fclose(file);


	String_View contents = sv_from_cstr(file_contents);
	
	Inst* inst = malloc(sizeof(Inst) * INST_BLOCK_SIZE);
	size_t noOfInst = 0;
	size_t capacityOfInst = INST_BLOCK_SIZE;

	while (contents.count > 0) {
		String_View line = sv_chop_by_delim(&contents, '\n');

		String_View trimmed_line = sv_trim(line);
		if (trimmed_line.count > 0 && *(trimmed_line.data) != '#') {
			//inst[noOfInt++] = compile_line(trimmed_line);

			printf("line: #"SV_Fmt"#\n", SV_Arg(trimmed_line));

			//if (capacityOfInst == noOfInst) {
			//	inst = realloc(inst, sizeof(Inst) * (capacityOfInst + INST_BLOCK_SIZE));
			//	capacityOfInst += INST_BLOCK_SIZE; 
			//}
		}
	}

	return inst;
}

// static Inst compile_line(String_View line) {
// 	String_View base_cmd = sv_chop_by_sv(line, sv_from_cstr(" \t"))
// 	if (sv_starts_with(line, sv_from_cstr("load"))) {
// 	}
// }

void write_inst_to_file(const char* file_path, Inst* inst, size_t count) {
}

int main(int argc, char* argv[]) {

	if (argc < 3) {
		printf("Usage: %s <file_path.src> <output_file.vm>\n", argv[0]);
		exit(1);
	}
	
	Inst* program = compile_file(argv[1]);

}
