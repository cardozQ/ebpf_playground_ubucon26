CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Wpedantic
TARGET := build/ebpf_workload
SOURCE := src/workload.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $<

run: all
	./$(TARGET)

clean:
	rm -rf build
