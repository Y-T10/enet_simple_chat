all: server client

server: server.cpp
	g++ -Wall $$(pkgconf --cflags libenet) -o $@ $^ $$(pkgconf --libs libenet)

client: client.cpp
	g++ -Wall $$(pkgconf --cflags libenet) -o $@ $^ $$(pkgconf --libs libenet)