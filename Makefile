
# Specify the compiler
CC = g++

#CCOPTS = -ansi -pedantic -Wall -g
CCOPTS = -g -Wall
LIBS = -pthread

# Make the source
all: host routercontroller util 

common.o : common.h common.cpp 
	$(CC) $(CCOPTS) -c common.cpp
 
message.o: message.h message.cpp
	$(CC) $(CCOPTS) -c message.cpp
     
host.o: host.h host.cpp
	$(CC) $(CCOPTS) -c host.cpp
	
router.o: router.h router.cpp
	$(CC) $(CCOPTS) -c router.cpp
	
content.o: content.h content.cpp
	$(CC) $(CCOPTS) -c content.cpp
	
host: host.cpp common.o message.o content.o
	$(CC) $(CCOPTS) $(LIBS) common.o message.o content.o host.cpp -o host
	
router: router.cpp common.o message.o content.o
	$(CC) $(CCOPTS) $(LIBS) common.o message.o content.o router.cpp -o router
	
routercontroller: routercontroller.cpp common.o message.o content.o router.o
	$(CC) $(CCOPTS) $(LIBS) common.o message.o content.o router.o routercontroller.cpp -o routercontroller
	
util: util.cpp common.o message.o content.o
	$(CC) $(CCOPTS) $(LIBS) common.o message.o content.o util.cpp -o util
	
clean :
	rm -f *.o main host router routercontroller util content_*
