TARGET_OS ?= qnx
srcdir := src

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

server: $(srcdir)/server.cpp
	$(CXX) $(CXXFLAGS) -o server $(srcdir)/server.cpp $(LIBS)

vehicle: $(srcdir)/vehicle.cpp $(srcdir)/messagehandler.cpp
	$(CXX) $(CXXFLAGS) -o vehicle $(srcdir)/vehicle.cpp $(srcdir)/messagehandler.cpp $(LIBS)

trafficlight: $(srcdir)/trafficlight.cpp $(srcdir)/messagehandler.cpp
	$(CXX) $(CXXFLAGS) -o trafficlight $(srcdir)/trafficlight.cpp $(srcdir)/messagehandler.cpp $(LIBS)

clean:
	rm -f server vehicle trafficlight

.PHONY: all clean