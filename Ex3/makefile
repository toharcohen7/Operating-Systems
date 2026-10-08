CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -pthread -Ihpp -MMD -MP
LDFLAGS := -pthread

# Source and Object files
SRC := $(wildcard cpp/*.cpp)
OBJ := $(SRC:cpp/%.cpp=cpp/%.o)
DEP := $(OBJ:.o=.d)

# Target executable
TARGET := ex3.out

.PHONY: all clean

all: $(TARGET)

# Linking the target executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDFLAGS)

# Compiling source files to object files
cpp/%.o: cpp/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJ) $(DEP) $(TARGET)

# Include dependency files
-include $(DEP)
