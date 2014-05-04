
# Specify the compiler
CC = g++

# Specify Boost build folder
BOOST_ROOT=boost_build
BOOST_INCLUDE_DIR=$(BOOST_ROOT)/include
BOOST_LIB_DIR=$(BOOST_ROOT)/lib
BOOST_LINK_FLAGS=-lboost_thread-mt
BOOST_LIB_THREAD=boost_thread
BOOST_LIB_CHRONO=boost_chrono
BOOST_LIB_DATETIME=boost_date_time
BOOST_LIB_SYSTEM=boost_system

#CCOPTS = -ansi -pedantic -Wall -g
CCOPTS = -g -Wall
LIBS = -pthread

# Make the source
all: host routercontroller util 

common.o : common.h common.cpp 
	$(CC) $(CCOPTS) -c common.cpp
 
message.o: message.h message.cpp
	$(CC) $(CCOPTS) -c message.cpp
	
router.o: router.h router.cpp  
	$(CC) $(CCOPTS) $(LIBS) -I $(BOOST_INCLUDE_DIR) -c router.cpp -L$(BOOST_LIB_DIR) -l$(BOOST_LIB_THREAD) -l$(BOOST_LIB_SYSTEM) -l$(BOOST_LIB_CHRONO) -l$(BOOST_LIB_DATETIME)
	
content.o: content.h content.cpp
	$(CC) $(CCOPTS) -c content.cpp

prt.o: prt.h prt.cpp
	$(CC) $(CCOPTS) -c prt.cpp 
	
rt.o: rt.h rt.cpp 
	$(CC) $(CCOPTS) -c rt.cpp 
	
host: host.h host.cpp common.o message.o content.o
	$(CC) $(CCOPTS) $(LIBS) -I $(BOOST_INCLUDE_DIR) common.o message.o content.o host.cpp -o host -L$(BOOST_LIB_DIR) -l$(BOOST_LIB_THREAD) -l$(BOOST_LIB_SYSTEM) -l$(BOOST_LIB_CHRONO) -l$(BOOST_LIB_DATETIME)
	
routercontroller: routercontroller.cpp common.o message.o content.o router.o  prt.o rt.o
	$(CC) $(CCOPTS) $(LIBS) common.o message.o content.o router.o  prt.o rt.o routercontroller.cpp -o routercontroller -L$(BOOST_LIB_DIR) -l$(BOOST_LIB_THREAD) -l$(BOOST_LIB_SYSTEM) -l$(BOOST_LIB_CHRONO) -l$(BOOST_LIB_DATETIME)
	
util: util.cpp common.o message.o content.o
	$(CC) $(CCOPTS) $(LIBS) common.o message.o content.o util.cpp -o util -L$(BOOST_LIB_DIR) -l$(BOOST_LIB_SYSTEM)
	
clean :
	rm -f *.o *.out main host routercontroller util content_* 
	rm -rf store*
