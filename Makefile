CXX			:= clang++-20
CXXFLAGS	:= -std=c++23 -O2 -g -DNDEBUG -Wall -Wextra -pedantic -stdlib=libstdc++ --gcc-toolchain=/usr
SRC			:= *.cpp
LLVMINC   := $(shell llvm-config-20 --includedir)
LLVMLIBS  := $(shell llvm-config-20 --ldflags --system-libs --libs core)
TARGET		:= toylang

.PHONY: compile \
		run \
		clean

compile: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -I$(LLVMINC) $(LLVMLIBS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o *.out
