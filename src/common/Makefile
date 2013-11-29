CXXFLAGS+=-g -pedantic -Wall -Wextra -std=c++98 -DBOOST_ASIO_DISABLE_THREADS -I..

OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(OBJECTS)

problem.o: problem.h problem.cpp
protocol.o: protocol.cpp protocol.h problem.h
cmd_options.o: cmd_options.cpp cmd_options.h
matrix.o: matrix.h matrix.cpp
connect.o: connect.cpp connect.h cmd_options.h


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

.PHONY: clean
clean:
	rm -f *.o
