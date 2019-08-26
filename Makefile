TARGET = libnetxcq.so
CPP = g++
CPPFLAGS = -g -std=c++11 -O0 -Wall -fPIC -Wno-format-security -I./
LINKFLAGS = -L./log -lcomlog -lpthread -lgflags -lprotobuf -L./libs/tcmalloc/ -ltcmalloc
SHAREDFLAGS = -shared
CPPS = $(wildcard ./*.cpp ./*/*.cpp ./*/*/*.cpp)
CPPOBJS = $(patsubst %.cpp, %.o, $(filter-out ./log/% ./example/%, $(CPPS)))
CCS = $(wildcard  ./packet/*.cc)
CCOBJS = $(patsubst %.cc, %.o, $(CCS))

so: $(CPPOBJS) $(CCOBJS)
	$(CPP) $(SHAREDFLAGS) $^ -o $(TARGET) $(LINKFLAGS)

install:
	$(shell sudo cp $(TARGET) /usr/local/lib)
	$(shell if [ ! -d /usr/share/libnetwork ]; then sudo mkdir /usr/share/libnetwork; fi)
	$(shell sudo cp -rf data /usr/share/libnetwork/)

test:
	@echo $(CPPS)
	@echo $(CCS)
	@echo $(CPPOBJS) $(CCOBJS)

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@

.cc.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	$(shell find ./ -name "*.o" |xargs rm )
	@rm $(TARGET)
