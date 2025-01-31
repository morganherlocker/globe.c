# Compiler
CXX = clang

# Compiler flags
CXXFLAGS = -std=c99 -Wall -Wextra -O3

# Target executable
TARGET = globe

# Source file
SRC = globe.c

LINT = clang-tidy --fix

FORMAT = clang-format -i

# Default target
all: clean $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

lint: format
	$(LINT) $(SRC) -- $(CXXFLAGS)

format:
	$(FORMAT) $(SRC)

# Clean up build files
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean lint format