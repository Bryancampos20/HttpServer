# Variables
CXX = g++
CXXFLAGS = -std=c++14 -pthread -g
GTEST_DIR = /usr/src/gtest
GTEST_LIB = -lgtest -lgtest_main

# Archivos fuente
SRC = src/request_handler.cpp src/server.cpp   # Para el servidor 
TEST_SRC = src/tests.cpp src/request_handler.cpp # Para los tests 

# Ejecutables
HTTP_SERVER = http_server
TESTS = tests

all: $(HTTP_SERVER) $(TESTS)

$(HTTP_SERVER): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST_LIB)

$(TESTS): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST_LIB)

clean:
	rm -f $(HTTP_SERVER) $(TESTS)

.PHONY: all clean
