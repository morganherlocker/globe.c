# Compiler
CXX = clang

# Compiler flags
CXXFLAGS = -Wall -Wextra -O3

# Target executable
TARGET = elevation

# Source file
SRC = elevation.c

# Object files
OBJS = $(SRCS:.cc=.o)

FORMAT = clang-tidy --fix

# Default target
all: clean $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

format:
	$(FORMAT) $(SRC) -- $(CXXFLAGS)

# Clean up build files
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean