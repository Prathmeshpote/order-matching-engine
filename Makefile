# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++17 -Wall -O2
# Source files
SRC = src/main.cpp src/matching_engine.cpp
# Output binary name
OUT = matching

# Default target
all: $(OUT)

# Build rule
$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

# Clean build
clean:
	rm -f $(OUT)
	