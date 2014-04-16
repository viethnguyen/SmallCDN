
# Specify the compiler
CC = g++

#CCOPTS = -ansi -pedantic -Wall -g
CCOPTS = -g -Wall
LIBS = -pthread

# Make the source
all: main
#all:	sender receiver

common.o : common.h common.cpp 
	$(CC) $(CCOPTS) -c common.cpp
 
message.o: message.h message.cpp
	$(CC) $(CCOPTS) -c message.cpp
     
#sender:  sender.cpp common.o message.o 
#	$(CC) $(CCOPTS) $(LIBS) common.o message.o sender.cpp -o sender

#receiver: receiver.cpp common.o message.o
#	$(CC) $(CCOPTS) $(LIBS) common.o message.o receiver.cpp -o receiver

main: main.cpp common.o message.o
	$(CC) $(CCOPTS) $(LIBS) common.o message.o main.cpp -o main
clean :
	rm -f common.o message.o main
