CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11
PREPROP = -D_NO_DELAY_

CPU_DIR = ../cpu
MEMORY_DIR = ../memory
TEST_DIR = .

SRCS = $(TEST_DIR)/test.cpp $(CPU_DIR)/MOS6502.cpp $(MEMORY_DIR)/Memory.cpp
HEADERS = $(CPU_DIR)/MOS6502.h $(MEMORY_DIR)/Memory.h

test: $(SRCS) $(HEADERS)
	$(CXX) -o $@ $(CXXFLAGS) $(PREPROP) $(SRCS)

clean:
	rm -rf ./test
