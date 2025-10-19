CXX			:= clang++-20
CXXFLAGS	:= -std=c++23 -O2 -g -DNDEBUG -Wall -Wextra -pedantic
SRC			:= ast.cpp lexer.cpp
TARGET		:= toylang

.PHONY: compile \
		run \
		clean

compile: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o *.out
