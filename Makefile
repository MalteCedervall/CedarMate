CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2

SRCS = Board.cpp Eval.cpp main.cpp Search.cpp
TEST_SRCS = Board.cpp Eval.cpp Search.cpp
TARGET = program
TEST_TARGET = test_moves

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): test_moves.cpp $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) -I. test_moves.cpp $(TEST_SRCS) -o $(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)
