CC = clang
CFLAGS = -Wall -std=c11 -pedantic -Wswitch-enum -Wextra -ggdb -g
LIBS = -lm

SRC_FILES = $(wildcard ./test/*.src)
COMPILED_FILES = $(wildcard ./test/*.vm)

.PHONY: all
all: vm compiler decompiler

vm: vm.o inst.o
	${CC} ${CFLAGS} ${LIBS} -o $@ $^

compiler: compiler.o inst.o
	${CC} ${CFLAGS} ${LIBS} -o $@ $^

decompiler: decompiler.o inst.o
	${CC} ${CFLAGS} ${LIBS} -o $@ $^

vm.o: ./src/vm.c
	${CC} ${CFLAGS} -c -I ./include -o $@ $<

inst.o: ./src/inst.c
	${CC} ${CFLAGS} -c -I ./include -o $@ $<

compiler.o: ./src/compiler.c
	${CC} ${CFLAGS} -c -I ./include -o $@ $<

decompiler.o: ./src/decompiler.c
	${CC} ${CFLAGS} -c -I ./include -o $@ $<

.PHONY: clean
clean:
	rm *.o vm compiler decompiler
	rm ./test/*.vm ./test/*.dbg ./test/*.out

compile: compiler $(SRC_FILES)
	$(foreach file, $(SRC_FILES), ./compiler $(file);)

decompile: decompiler $(COMPILED_FILES)
	$(foreach file, $(COMPILED_FILES), ./decompiler $(file);)

run: vm $(COMPILED_FILES)
	$(foreach file, $(COMPILED_FILES), ./vm $(file) &> $(file).out ;)


.PHONY: test
test: compile decompile run
