CXX=g++
CXXFLAGS=-std=c++11 -O2 -Wall -g
BUILD_DIR = ./bin
TARGET:=./bin/myserver

SOURCE= main.cpp ./pool/threadpool.h ./lock/* ./epoll/epoller.cpp ./webserver/webserver.cpp ./buffer/buffer.cpp ./http/*.cpp ./log/log.cpp ./config/config.cpp ./timer/timer.cpp

$(TARGET):$(SOURCE)
	$(CXX) $(CXXFLAGS)  $(SOURCE) -o $(TARGET) -pthread

build: $(BUILD_DIR)/myserver

mk_dir:
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi

all: mk_dir build

clean:
	cd bin; rm -rf *

run:
	cd ./bin ; ./myserver

runlog2timeout2s:
	cd ./bin; ./myserver -l 2 -o 2000