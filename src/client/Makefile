CXXFLAGS=-g -pedantic -Wall -Wextra -std=c++98

TARGET=mdpc
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	../../lib/msgpack/libtool --mode=link --tag=CXX $(CXX) $(LDFLAGS) $< ../../lib/msgpack/src/libmsgpack.la -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
