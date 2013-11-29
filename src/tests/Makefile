CXXFLAGS+=-g -pedantic -Wall -Wextra -std=c++98 -DBOOST_ASIO_DISABLE_THREADS -I.. -Wno-c++11-long-long

#OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) \
    ../common/matrix.o

matrix_test: matrix_test.cpp ../common/matrix.o 
	$(CXX) $(CXXFLAGS) matrix_test.cpp ../common/matrix.h ../common/matrix.cpp ../common/location.h -o matrix_test

BINARIES=matrix_test

.PHONY: clean
clean:
	rm -f *.o $(BINARIES) $(TARGET) 
	cd ../common; make clean
