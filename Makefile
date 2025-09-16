CXX = g++
CXXFLAGS = -Wall -std=c++17 -g
TARGET = account

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET)