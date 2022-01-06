CC = cc
CFLAGS = -Wall -std=c11 -pedantic -Wswitch-enum -g
LIBS = -lm

vm: vm.o instructions.o
	${CC} ${CFLAGS} ${LIBS} -o vm vm.o instructions.o

vm.o: ./src/vm.c ./include/instructions.h
	${CC} ${CFLAGS} -c -I ./include -o vm.o ./src/vm.c

instructions.o: ./src/instructions.c
	${CC} ${CFLAGS} -c -I ./include -o instructions.o ./src/instructions.c

clean:
	rm *.o vm
