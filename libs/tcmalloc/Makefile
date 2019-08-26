TARGET = libtcmalloc.a
CPP = g++
C = gcc
CFLAGS = -g -O0 -Wall -fPIC -I./src/ -I./src/gperftools
CPPFLAGS = $(CFLAGS) -std=c++11
SAFEFLAGS = -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
WARNINGFLAGS = -Wno-sign-compare
LINKFLAGS = -lpthread
CCS = $(wildcard ./src/*.cc ./src/*.c ./src/*/*.cc ./src/*/*.c)
COBJS = $(patsubst %.c, %.o, $(CCS))
CCOBJS = $(patsubst %.cc, %.o, $(COBJS))

a: $(CCOBJS)
	ar rc $(TARGET) $(CCOBJS)

install:
	$(shell sudo cp $(TARGET) /usr/local/lib)
	$(shell if [ ! -d /usr/share/libnetwork ]; then sudo mkdir /usr/share/libnetwork; fi)
	$(shell sudo cp -rf data /usr/share/libnetwork/)

test:
	@echo $(CCS)
	@echo $(CCOBJS)

.cc.o:
	$(CPP) $(CPPFLAGS) $(SAFEFLAGS) $(WARNINGFLAGS) -c $< -o $@

.c.o:
	$(C) $(CFLAGS) $(SAFEFLAGS) $(WARNINGFLAGS) -c $< -o $@

clean:
	$(shell find ./ -name "*.o" |xargs rm )
	@rm $(TARGET)
