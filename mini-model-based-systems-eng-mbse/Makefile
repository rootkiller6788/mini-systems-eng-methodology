CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude -Wno-misleading-indentation
LDFLAGS = -lm
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))
TEST_SRC = tests/test_mbse.c
TEST_BIN = build/test_mbse
EXAMPLE_SRCS = $(wildcard examples/*.c)
EXAMPLE_BINS = $(patsubst examples/%.c,build/%,$(EXAMPLE_SRCS))

.PHONY: all builddir clean test examples demo

all: builddir build/libmbse.a

builddir:
	@mkdir -p build

build/libmbse.a: $(OBJS)
	$(AR) rcs $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: all
	$(CC) $(CFLAGS) -o $(TEST_BIN) $(TEST_SRC) build/*.o $(LDFLAGS)
	./$(TEST_BIN)

examples: all $(EXAMPLE_BINS)
	@echo All examples built.

build/%: examples/%.c
	$(CC) $(CFLAGS) -o $@ $< build/*.o $(LDFLAGS)

demo: examples
	@echo === Running mbse_demo ===
	./build/mbse_demo
	@echo 
	@echo === Running example2_requirements ===
	./build/example2_requirements
	@echo 
	@echo === Running example3_trade_study ===
	./build/example3_trade_study

clean:
	rm -rf build
