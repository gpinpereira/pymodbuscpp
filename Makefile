# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall

PYINCLUDES = $(shell python3 -m pybind11 --includes)

# Directories
INCLUDES = -I/usr/local/include/modbus -I./lib/ -I$(shell python3 -m pybind11 --includes) 

SRC_DIR = .
SRC_FILES = main.cpp \
			lib/volatile_.cpp \
			lib/net_.cpp \
			lib/memory_.cpp \
            lib/exception_.cpp \
            lib/math_.cpp \
			lib/time_.cpp \
			lib/random_.cpp \
			lib/string_.cpp \
			lib/mutex_.cpp \
			lib/thread_.cpp \
			lib/modbus_.cpp

OBJ_FILES = $(SRC_FILES:.cpp=.o)
TARGET = program

# Libraries
LIBS = -lpthread -lmodbus -lpython3.10


# Build target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(OBJ_FILES) -o $(TARGET) $(LIBS) 

# Compile each source file into an object file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(PYINCLUDES) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJ_FILES) $(TARGET)
