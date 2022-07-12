SRCEXTS = .c .C .cc .cpp .c++ .cxx .cp
HDREXTS = .h .H .hh .hpp .h++ .hxx .hp
CC = gcc
CXX = g++
CTAGS = ctags

INCLUDES=-I/usr/local/include -I/usr/include -I${CPPSDK}
LIBS=-L/usr/local/lib -L/usr/lib -ltcmalloc -lcurl -lmagic
CFLAGS = -g -Wall -fPIC -c $(INCLUDES) -std=c++0x
OFLAGS = -g -Wall $(LIBS)

CPPSDK=./
HTTP=$(CPPSDK)/http
UTILS=$(CPPSDK)/utils
BASE64=$(CPPSDK)/base64
DIGEST=$(CPPSDK)/digest
HMAC=$(CPPSDK)/hmac
ERROR=$(CPPSDK)/errno
CONFIG=$(CPPSDK)/config
API=$(CPPSDK)/api
SRCS = $(wildcard $(HTTP)/*.cpp $(UTILS)/*.cpp $(BASE64)/*.cpp $(DIGEST)/*.cpp $(HMAC)/*.cpp $(ERROR)/*.cpp $(API)/*.cpp $(CONFIG)/*.cpp)
OBJS = $(addsuffix .o, $(basename $(SRCS)))
HEADS = $(addsuffix .h, $(basename $(SRCS)))

all: $(OBJS)
	ar -r libufilecppsdk.a $^

%.o:%.cc
	$(CXX) $(CFLAGS) -o $@ $<

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<

%.o:%.cpp
	$(CXX) $(CFLAGS) -o $@ $<

clean :
	find . -name '*.o' | xargs rm -f
	rm -rf libufilecppsdk.a
