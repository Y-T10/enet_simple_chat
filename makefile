ENET_FLAGS:=$$(pkgconf --cflags libenet)
ENET_LDLIBS:=$$(pkgconf --libs libenet)
CXXFLAGS:=-Wall -std=gnu++20 $(ENET_FLAGS)
LDLIBS:=-lpthread $(ENET_LDLIBS)
SRCDIR:=./src
OBJDIR:=./.obj

all: server client

server: $(OBJDIR)/server.o $(OBJDIR)/enet_host.o $(OBJDIR)/basic_enet.o $(OBJDIR)/SigEvent.o $(OBJDIR)/enet_packet_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

client: $(OBJDIR)/client.o $(OBJDIR)/enet_client.o $(OBJDIR)/basic_enet.o $(OBJDIR)/console_io.o $(OBJDIR)/enet_packet_stream.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $^ $(LDLIBS)

clean:
	-@rm $(OBJDIR)/*.o
	-@rm server
	-@rm client