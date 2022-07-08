CXX=g++
CXXFLAGS=-std=c++14 -O2 -Wall -g
BUILD_DIR = ./bin
TARGET:=./bin/myserver

SOURCE= main.cpp

$(TARGET):$(SOURCE)
	$(CXX) $(CXXFLAGS)  $(SOURCE) -o $(TARGET) -pthread

build: $(BUILD_DIR)/myserver

all:  build

clean:
	cd bin; rm *