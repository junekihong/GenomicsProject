CXXFLAGS+=-g -pedantic -Wall -Wextra -std=c++98 -DBOOST_ASIO_DISABLE_THREADS -I.. -Wno-long-long -I../../lib/msgpack/src

#OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) \
    ../common/matrix.o

matrix_test: matrix_test.cpp ../common/matrix.o ../client/fasta.o
	$(CXX) $(CXXFLAGS) matrix_test.cpp ../common/matrix.o ../client/fasta.o ../../lib/msgpack/src/.libs/libmsgpack.a -o matrix_test

BINARIES=matrix_test

.PHONY: clean
clean:
	rm -f *.o $(BINARIES)
	cd ../common; make clean
