
#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stdint.h>
#include <sv.h>
#include <inst.h>

#define INST_BLOCK_SIZE 1024

Inst* compile_file(const char* file_path);

void write_inst_to_file(const char* file_path, Inst* inst, size_t count);

#endif
