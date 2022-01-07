CC = cc
CFLAGS = -Wall -std=c11 -pedantic -Wswitch-enum -g
LIBS = -lm

vm: vm.o inst.o executors.o
	${CC} ${CFLAGS} ${LIBS} -o vm vm.o inst.o executors.o

compiler: compiler.o
	${CC} ${CFLAGS} ${LIBS} -o compiler compiler.o

vm.o: ./src/vm.c
	${CC} ${CFLAGS} -c -I ./include -o vm.o ./src/vm.c

inst.o: ./src/inst.c
	${CC} ${CFLAGS} -c -I ./include -o inst.o ./src/inst.c

compiler.o: ./src/compiler.c
	${CC} ${CFLAGS} -c -I ./include -o compiler.o ./src/compiler.c

executors.o: ./src/executors.c
	${CC} ${CFLAGS} -c -I ./include -o executors.o ./src/executors.c

clean:
	rm *.o vm compiler
