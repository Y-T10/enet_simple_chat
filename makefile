ENET_FLAGS:=$$(pkgconf --cflags libenet)
ENET_LDLIBS:=$$(pkgconf --libs libenet)
CXXFLAGS:=-Wall -std=c++20 $(ENET_FLAGS)
LDLIBS:=-lpthread $(ENET_LDLIBS)

all: server client

server: server.o enet_send.o
	g++ $(CXXFLAGS) -o $@ $^ $(LDLIBS)

client: client.o enet_send.o
	g++ $(CXXFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	g++ $(CXXFLAGS) -Wall -c $^ $(LDLIBS)

clean:
	-@rm *.o
	-@rm server.exe
	-@rm client.exe