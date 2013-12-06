CXXFLAGS+=-g -Wall -Wextra -std=c++98 -DBOOST_ASIO_DISABLE_THREADS -I.. -I ../../lib/msgpack/src -Wno-long-long
CXXFLAGS+=-DDEBUG

OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(OBJECTS)

problem.o: problem.h problem.cpp
protocol.o: protocol.cpp protocol.h problem.h
cmd_options.o: cmd_options.cpp cmd_options.h
matrix.o: matrix.h matrix.cpp pair_location_value.h
connect.o: connect.cpp connect.h cmd_options.h
listen.o: listen.cpp listen.h


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

.PHONY: clean
clean:
	rm -f *.o
