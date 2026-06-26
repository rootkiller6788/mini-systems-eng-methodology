CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude -Wno-misleading-indentation
LDFLAGS = -lm
LIB = build/libstakeholder.a
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))
TEST_SRC = tests/test_stakeholder.c
TEST_BIN = build/test_stakeholder
EXAMPLES_SRC = $(wildcard examples/*.c)
EXAMPLES_BIN = $(patsubst examples/%.c,build/ex_%,$(EXAMPLES_SRC))

all: builddir $(LIB)

builddir:
	@mkdir -p build

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: builddir $(LIB)
	$(CC) $(CFLAGS) -o $(TEST_BIN) $(TEST_SRC) -Lbuild -lstakeholder $(LDFLAGS)
	./$(TEST_BIN)

examples: builddir $(LIB) $(EXAMPLES_BIN)

build/ex_%: examples/%.c $(LIB)
	$(CC) $(CFLAGS) -o $@ $< -Lbuild -lstakeholder $(LDFLAGS)
	@echo "  Running: $@"
	@./$@

demos: builddir $(LIB)
	$(CC) $(CFLAGS) -o build/demo demos/demo.c -Lbuild -lstakeholder $(LDFLAGS)
	./build/demo

clean:
	rm -rf build
