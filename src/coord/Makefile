CXXFLAGS+=-DDEBUG
CXXFLAGS+=-g -pedantic -Wall -Wextra -std=c++98 -I.. -DBOOST_ASIO_DISABLE_THREADS

TARGET=mdpl
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) ../common/cmd_options.o \
    ../common/listen.o ../common/protocol.o ../common/matrix.o ../common/connect.o

LDFLAGS=-lboost_program_options -lboost_system

all: $(TARGET)

actions.o: actions.cpp protocol.h
cmd_options.h: ../common/cmd_options.h
cmd_options.o: cmd_options.cpp cmd_options.h
protocol.h: ../common/problem.h ../common/solution.h
protocol.o: protocol.cpp protocol.h ../common/protocol.h ../common/util.h
mdpl.o: mdpl.cpp cmd_options.h protocol.h ../common/protocol.h ../common/listen.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
