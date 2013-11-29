CXXFLAGS=-g -pedantic -Wall -Wextra -std=c++98

TARGET=mdps
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	../../lib/msgpack/libtool --mode=link --tag=CXX $(CXX) $(LDFLAGS) $^ -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
