CXX = g++                   
CXXFLAGS = -Iinclude -pthread 
SRC = src/server.cpp src/request_handler.cpp 
OBJ = $(SRC:.cpp=.o)       
TARGET = http_server        

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
