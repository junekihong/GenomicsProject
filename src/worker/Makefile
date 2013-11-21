CXX=clang++
CXXFLAGS=-g -pedantic -Wall -Wextra -std=c++98 -DBOOST_ASIO_DISABLE_THREADS

TARGET=mdpw
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
LDFLAGS+=-lboost_program_options -lboost_system

all: $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	echo "$(OBJECTS)"
	../../lib/msgpack/libtool --mode=link --tag=CXX $(CXX) $(LDFLAGS) $^ ../../lib/msgpack/src/libmsgpack.la -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
