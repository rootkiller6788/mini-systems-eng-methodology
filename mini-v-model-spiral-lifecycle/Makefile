CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude -I../../include
LDFLAGS = -lm
LIB = build/libvmodel_spiral_lifecycle.a
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

.PHONY: all builddir clean test demo examples

all: builddir $(LIB)

builddir:
	@mkdir -p build

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

build/%.o: src/%.c | builddir
	$(CC) $(CFLAGS) -c $< -o $@

test: $(LIB)
	$(CC) $(CFLAGS) tests/test_lifecycle.c -Lbuild -lvmodel_spiral_lifecycle $(LDFLAGS) -o build/test_lifecycle.exe
	./build/test_lifecycle.exe

examples: ex1 ex2 ex3

ex1: $(LIB)
	$(CC) $(CFLAGS) examples/example1_vmodel.c -Lbuild -lvmodel_spiral_lifecycle $(LDFLAGS) -o build/example1.exe
ex2: $(LIB)
	$(CC) $(CFLAGS) examples/example2_spiral_risk.c -Lbuild -lvmodel_spiral_lifecycle $(LDFLAGS) -o build/example2.exe
ex3: $(LIB)
	$(CC) $(CFLAGS) examples/example3_full_lifecycle.c -Lbuild -lvmodel_spiral_lifecycle $(LDFLAGS) -o build/example3.exe

demo: examples
	@echo "=== Example 1: V-Model ===" && ./build/example1.exe
	@echo "=== Example 2: Spiral + Risk ===" && ./build/example2.exe
	@echo "=== Example 3: Full Lifecycle ===" && ./build/example3.exe

clean:
	rm -rf build
