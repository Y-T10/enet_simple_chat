ENET_FLAGS:=$$(pkgconf --cflags libenet)
ENET_LDLIBS:=$$(pkgconf --libs libenet)
CXXFLAGS:=-Wall -std=c++20 $(ENET_FLAGS)
LDLIBS:=-lpthread $(ENET_LDLIBS)

all: server client

server: server.o enet_host.o basic_enet.o SigEvent.o enet_packet_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

client: client.o enet_client.o basic_enet.o  console_io.o enet_packet_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ $(LDLIBS)

clean:
	-@rm *.o
	-@rm server
	-@rm client