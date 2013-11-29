CXXFLAGS=-g -pedantic -Wall -Wextra -std=c++98

TARGET=mdpc
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
