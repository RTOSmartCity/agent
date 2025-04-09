CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = # -lsocket

all: server client

server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp $(LIBS)

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp $(LIBS)

clean:
	rm -f server client

.PHONY: all clean