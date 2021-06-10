CC = g++ -std=c++11

all: console.out system.out router.out

console.out: console.o
	${CC} console.o -o console.out

system.out: system.o
	${CC} system.o -o system.out

router.out: router.o
	${CC} router.o -o router.out

console.o: console.cpp
	${CC} -c console.cpp

system.o: system.cpp
	${CC} -c system.cpp

router.o: router.cpp
	${CC} -c router.cpp

.PHONY: clean
clean:
	rm *.o
	rm *.out