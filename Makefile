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

LINT = clang-tidy --fix

FORMAT = clang-format -i

# Default target
all: clean $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

lint:
	$(LINT) $(SRC) -- $(CXXFLAGS)

format:
	$(FORMAT) $(SRC)

# Clean up build files
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean