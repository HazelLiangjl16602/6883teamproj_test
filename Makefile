# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17   # Notice: now using C++17 standard

# Target executable name
TARGET = EODMarketData

# Source files
SRCS = main.cpp RetrieveData.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Linking step (link object files and link to libcurl)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -lcurl

# Compile each .cpp file into .o file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(TARGET)

# Run (optional)
run: $(TARGET)
	./$(TARGET)