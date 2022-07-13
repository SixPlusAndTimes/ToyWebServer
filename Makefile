CXX=g++
CXXFLAGS=-std=c++11 -O1 -Wall -g
BUILD_DIR = ./bin
TARGET:=./bin/myserver

SOURCE= main.cpp ./pool/threadpool.h ./lock/lock.h ./epoll/epoller.cpp ./webserver/webserver.cpp

$(TARGET):$(SOURCE)
	$(CXX) $(CXXFLAGS)  $(SOURCE) -o $(TARGET) -pthread

build: $(BUILD_DIR)/myserver

all:  build

clean:
	cd bin; rm *

run:
	./bin/myserver 10000