CXXFLAGS=-g -pedantic -Wall -Wextra -std=c++98 -DBOOST_ASIO_DISABLE_THREADS -I..

OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(OBJECTS)

problem.o: problem.h problem.cpp

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

.PHONY: clean
clean:
	rm -f *.o
