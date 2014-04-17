
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
     
host.o: host.h host.cpp
	$(CC) $(CCOPTS) -c host.cpp
	
content.o: content.h content.cpp
	$(CC) $(CCOPTS) -c content.cpp
	
#sender:  sender.cpp common.o message.o 
#	$(CC) $(CCOPTS) $(LIBS) common.o message.o sender.cpp -o sender

#receiver: receiver.cpp common.o message.o
#	$(CC) $(CCOPTS) $(LIBS) common.o message.o receiver.cpp -o receiver

main: main.cpp common.o message.o host.o content.o
	$(CC) $(CCOPTS) $(LIBS) host.o common.o message.o content.o main.cpp -o main
clean :
	rm -f common.o message.o host.o content.o main
