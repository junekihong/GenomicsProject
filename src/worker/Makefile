CXXFLAGS=-g -pedantic -Wall -Wextra -std=c++98 -DBOOST_ASIO_DISABLE_THREADS -I.. -Wno-c++11-long-long

TARGET=mdpw
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) ../common/problem.o ../common/protocol.o
LDFLAGS+=-lboost_program_options -lboost_system

all: $(TARGET)

protocol.h: ../common/problem.h ../common/solution.h ../common/matrix.h
	touch protocol.h # hack to get this depndency to matter
../common/problem.o: ../common/problem.h ../common/problem.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp -o ../common/problem.o
protocol_impl.h: protocol.h
worker.h: protocol.h
protocol_impl.o: protocol_impl.cpp protocol_impl.h ../common/protocol.h
mdpw.o: mdpw.cpp cmd_options.h protocol.h protocol_impl.h worker.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	cd ../common; make
	$(CXX) $(LDFLAGS) $^ -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
	cd ../common; make clean
