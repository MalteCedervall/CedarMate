CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2

SRCS = Board.cpp Eval.cpp main.cpp Search.cpp
TARGET = program

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
