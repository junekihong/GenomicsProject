CXXFLAGS=-Wall -Wextra -std=c++98  -DBOOST_ASIO_DISABLE_THREADS -I.. -I ../../lib/msgpack/src -Wno-long-long
#CXXFLAGS+=-g -DDEBUG

TARGET=mdpc
OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) ../common/cmd_options.o ../common/connect.o \
    ../common/matrix.o ../common/protocol.o
LDFLAGS=-lboost_system -g

all: $(TARGET)

mdpc.o: mdpc.cpp fasta.h \
    ../common/cmd_options.h ../common/connect.h ../common/protocol.h ../common/util.h
fasta.o: fasta.cpp fasta.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(TARGET) : $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) ../../lib/msgpack/src/.libs/libmsgpack.a -o $(TARGET)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
