CXXFLAGS=-g -pedantic -Wall -Wextra -std=c++98 -I..

TARGET=mdpc
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) ../common/cmd_options.o ../common/connect.o
LDFLAGS=-lboost_system

all: $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
