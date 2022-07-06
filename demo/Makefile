SRCEXTS = .c .C .cc .cpp .c++ .cxx .cp
HDREXTS = .h .H .hh .hpp .h++ .hxx .hp
CC = gcc
CXX = g++
MAKE = make
CTAGS = ctags

INCLUDES=-I/usr/local/include -I/usr/include -I../ucloud
LIBS=-L/usr/local/lib -L/usr/lib -ltcmalloc -lcurl -lmagic -ljson-c -L../ucloud -lufilecppsdk
CFLAGS = -g -Wall -fPIC -c $(INCLUDES) -std=c++0x
OFLAGS = -g -Wall $(LIBS)

SRCS = $(wildcard ./*.cpp)
OBJS = $(addsuffix .o, $(basename $(SRCS)))
HEADS = $(addsuffix .h, $(basename $(SRCS)))

APP=demo
$(APP): $(OBJS)
	$(MAKE) -C ../ucloud/
	$(CXX) $^ -o $@ $(OFLAGS)

%.o:%.cc
	$(CXX) $(CFLAGS) -o $@ $<

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<

%.o:%.cpp
	$(CXX) $(CFLAGS) -o $@ $<

clean :
	rm -f $(APP) *.o
	$(MAKE) -C ../ucloud/ clean
