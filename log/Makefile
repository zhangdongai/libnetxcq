TARGET = libcomlog.a
CPP = g++
CPPFLAGS = -g -std=c++11 -O0 -fPIC -Wall -I./ -I../
LINKFLAGS = -lpthread
CPPS = $(wildcard ./*.cpp)
CPPOBJS = $(patsubst %.cpp,%.o,$(CPPS))

a: $(CPPOBJS)
	ar rcs $(TARGET) $(CPPOBJS)

test:
	@echo $(CPPS)
	@echo $(CPPOBJS)

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	$(shell find ./ -name "*.o" |xargs rm )
	@rm $(TARGET)
