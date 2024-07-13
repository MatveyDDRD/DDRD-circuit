# This makefile automatically builds all files in src/

FLAGS = -Wpointer-integer-compare $(shell pkg-config --cflags gtk4)
LIBS = $(shell pkg-config --libs gtk4)
CC = gcc

EXCLUDE_FILES :=

# Определение исходных файлов с исключением из списка EXCLUDE_FILES
SRCS := $(filter-out $(EXCLUDE_FILES), $(wildcard src/*.c))
OBJS := $(patsubst src/%.c, build/obj/%.o, $(SRCS))

# Основная цель: компилировать программу и запустить её
all: install

# Цель для компиляции и сборки программы
install: DDRD-circuit

DDRD-circuit: $(OBJS)
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

# Правило для компиляции любого исходного файла в объектный файл
build/obj/%.o: src/%.c src/*.h
	$(CC) $(FLAGS) -c $< -o $@

# Цель для запуска программы
run: DDRD-circuit
# 	gtimeout 5 ./DDRD-circuit
	./DDRD-circuit

# Цель для очистки (удаления) временных файлов и исполняемого файла
clean:
	rm -f build/obj/*.o DDRD-circuit