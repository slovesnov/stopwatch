# mingw32-make -j $(nproc)

# 1. Compiler and flags settings
ASLOV    := ../../aslov/aslov/src/aslov
CXX      := g++
CXXFLAGS := -O3 -DNDEBUG -Wall -std=c++23 -I$(ASLOV) $(shell pkg-config --cflags gtk+-3.0)
LIBS     := $(shell pkg-config --libs gtk+-3.0) -lcurl
TARGET   := stopwatch

# Windows-specific flags (hides the console window)
ifeq ($(OS),Windows_NT)
    LDFLAGS += -mwindows
endif

# 2. Automatic file discovery
SRCS     := $(wildcard src/*.cpp) $(wildcard $(ASLOV)/*.cpp)
OBJS     := $(SRCS:.cpp=.o)

EXTRA_OBJS := $(TARGET)/icon/app.o

# 3. Main target
all: $(TARGET)

# Links the executable from the object files (LDFLAGS placed before LIBS)
$(TARGET): $(OBJS) $(EXTRA_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(EXTRA_OBJS) $(LDFLAGS) $(LIBS)

# Rule to compile each .cpp file into a .o file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. Auxiliary commands
# Cleans the project by removing temporary build files
clean:
	rm -f $(OBJS) $(TARGET).exe

# Declares targets that do not represent actual files on disk
.PHONY: all clean
