CXXFLAGS=-Wall -Wextra -std=c++98 -I.. -DBOOST_ASIO_DISABLE_THREADS -I ../../lib/msgpack/src -Wno-long-long
#CXXFLAGS+=-g -DDEBUG

TARGET=mdps
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) ../common/listen.o ../common/matrix.o\
    ../common/protocol.o
LDFLAGS+=-lboost_program_options -lboost_system -lboost_filesystem -g

all: $(TARGET)

cmd_options.o: cmd_options.cpp cmd_options.h ../common/cmd_options.h
genomeOps.o: genomeOps.cpp genomeOps.h
mdps.o: mdps.cpp cmd_options.h genomeOps.h ../common/listen.h solutionOps.h ../common/protocol.h
solutionOps.o: solutionOps.cpp solutionOps.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) ../../lib/msgpack/src/.libs/libmsgpack.a -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
