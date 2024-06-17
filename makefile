CXX=g++
CXXFLAGS=-std=c++17 -g -pedantic -Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer
LDLIBS=

TOP=test.cpp
SRCS=webserver.cpp loadbalancer.cpp 
DEPS=common.cpp TCPRequestChannel.cpp
OBJS=$(DEPS:%.cpp=%.o)
OBJS2=$(SRCS:%.cpp=%.o)
LAST=$(TOP:%.cpp=%.exe)


all: clean $(LAST)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.exe: %.cpp $(OBJS) $(OBJS2)
	$(CXX) $(CXXFLAGS) -o $(patsubst %.exe,%,$@) $^ $(LDLIBS)

.PHONY: clean collect-minfo test

clean:
	rm -f server client *.tst *.o *.csv received/*

collect-minfo:
	uname -a > minfo.txt && lscpu >> minfo.txt && git add minfo.txt && git commit -m "Machine information"

test: all

do-last: $(OBJS2) $(LAST)