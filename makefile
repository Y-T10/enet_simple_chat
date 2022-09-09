ENET_FLAGS:=$$(pkgconf --cflags libenet)
ENET_LDLIBS:=$$(pkgconf --libs libenet)
CXXFLAGS:=-Wall -std=c++20 $(ENET_FLAGS)
LDLIBS:=-lpthread $(ENET_LDLIBS)

all: server client

server: server2.o enet_host.o basic_enet.o SigEvent.o
	g++ $(CXXFLAGS) -o $@ $^ $(LDLIBS)

client: client2.o enet_client.o basic_enet.o  console_io.o
	g++ $(CXXFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	g++ $(CXXFLAGS) -Wall -c $^ $(LDLIBS)

clean:
	-@rm *.o
	-@rm server
	-@rm client