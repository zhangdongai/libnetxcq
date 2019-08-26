CPP = g++
CPPFLAGS = -g -std=c++11 -O0 -fPIC -Wall -Wno-format-security -I../
LINKFLAGS = -lnetxcq -lpthread -lprotobuf

server: server.o
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LINKFLAGS)

client: client.o ../packet/connector.pb.o
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LINKFLAGS)

test:
	@echo $(CPPS)
	@echo $(CPPOBJS)

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@

.cc.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	$(shell find ./ -name "*.o" |xargs rm )
	@rm server
	@rm client
