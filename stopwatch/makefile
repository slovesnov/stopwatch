# mingw32-make -j $(nproc)

# 1. Настройки компилятора и флагов
ASLOV    := ../../aslov/aslov/src/aslov
CXX      := g++
CXXFLAGS := -O3 -DNDEBUG -Wall -std=c++23 -I$(ASLOV) $(shell pkg-config --cflags gtk+-3.0)
LIBS     := $(shell pkg-config --libs gtk+-3.0) -lcurl
TARGET   := stopwatch

# Специфичные флаги для Windows (скрываем консоль)
ifeq ($(OS),Windows_NT)
    LDFLAGS += -mwindows
endif

# 2. Автоматический поиск файлов
SRCS     := $(wildcard src/*.cpp) $(wildcard $(ASLOV)/*.cpp)
OBJS     := $(SRCS:.cpp=.o)

EXTRA_OBJS := $(TARGET)/icon/app.o

# 3. Главный таргет
all: $(TARGET)

# Сборка исполняемого файла (флаг LDFLAGS добавлен перед LIBS)
$(TARGET): $(OBJS) $(EXTRA_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(EXTRA_OBJS) $(LDFLAGS) $(LIBS)

# Правило для компиляции каждого файла .cpp в .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. Вспомогательные команды
clean:
	rm -f $(OBJS) $(TARGET).exe $(TARGET)

.PHONY: all clean
