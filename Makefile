TARGET_OS ?= notqnx

ifeq ($(TARGET_OS), qnx)
CXX = q++
CXXFLAGS = -std=c++17 -Wall -Wextra -Vgcc_ntoaarch64le
LIBS = -lsocket
else
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = # -lsocket
endif

all: server vehicle trafficlight

server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp $(LIBS)

vehicle: vehicle.cpp messagehandler.cpp
	$(CXX) $(CXXFLAGS) -o vehicle vehicle.cpp messagehandler.cpp $(LIBS)

trafficlight: trafficlight.cpp messagehandler.cpp
	$(CXX) $(CXXFLAGS) -o trafficlight trafficlight.cpp messagehandler.cpp $(LIBS)

clean:
	rm -f server vehicle trafficlight

.PHONY: all clean