# QNX-specific Makefile for aarch64
CXX = q++ -Vgcc_ntoaarch64le
CXXFLAGS = -D_QNX_SOURCE -Wall -Wextra
LIBS = -Wl,-Bstatic -lsocket -Wl,-Bdynamic -lc

all: server client

server: server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

client: client.cpp
	q++ $(CXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f server client

.PHONY: all clean