CXXFLAGS=-g -pedantic -Wall -Wextra -std=c++98 -I..

TARGET=mdps
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
LDFLAGS+=-lboost_program_options

all: $(TARGET)

cmd_options.o: cmd_options.cpp cmd_options.h ../common/cmd_options.h
mdps.o: mdps.cpp cmd_options.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
