# ===== Makefile =====

# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Executable name
TARGET = final_test

# Source files
SRCS = Main.cpp \
       RetrieveData.cpp \
       Stock.cpp \
       GroupedStock.cpp \
       BootStrap.cpp \
       Menu.cpp \
       GNUPlot.cpp    

# Header dependencies (optional, good practice)
HDRS = RetrieveData.h Stock.h GroupedStock.h BootStrap.h GNUPlot.h

# Libraries (for curl)
LIBS = -lcurl

# Build rules
all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

# Clean rules
clean:
	rm -f $(TARGET)

# ===== End =====
