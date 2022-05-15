all: server client

server: server.o enet_send.o
	g++ -Wall $$(pkgconf --cflags libenet) -o $@ $^ $$(pkgconf --libs libenet)

client: client.o enet_send.o
	g++ -Wall $$(pkgconf --cflags libenet) -o $@ $^ $$(pkgconf --libs libenet)

%.o: %.cpp
	g++ -Wall -c $^