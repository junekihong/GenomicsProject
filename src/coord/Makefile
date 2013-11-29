CXXFLAGS+=-g -pedantic -Wall -Wextra -std=c++98 -I..

TARGET=mdpl
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) ../common/cmd_options.o
LDFLAGS=-lboost_program_options -lboost_system

all: $(TARGET)

cmd_options.h: ../common/cmd_options.h
cmd_options.o: cmd_options.cpp cmd_options.h
mdpl.o: mdpl.cpp cmd_options.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
