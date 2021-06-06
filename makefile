CC = g++ -std=c++11

all: client.out server.out group_server.out router.out

client.out: client.o
	${CC} client.o -o client.out

server.out: server.o
	${CC} server.o -o server.out

group_server.out: group_server.o
	${CC} group_server.o -o group_server.out

router.out: router.o
	${CC} router.o -o router.out

client.o: client.cpp
	${CC} -c client.cpp

server.o: server.cpp
	${CC} -c server.cpp

group_server.o: group_server.cpp
	${CC} -c group_server.cpp

router.o: router.cpp
	${CC} -c router.cpp

.PHONY: clean
clean:
	rm *.o
	rm *.out