CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinclude -Wno-misleading-indentation
LDFLAGS = -lm
LIB = build/libcst_flood.a
SRCS = src/cst_core.c src/cst_tsi.c src/cst_sosm.c src/cst_boundary.c src/cst_churchman.c src/cst_intervention.c src/cst_ethics.c src/cst_learning.c src/cst_power.c src/cst_dialogue.c src/cst_evaluation.c
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))
TEST_SRC = tests/test_cst.c
TEST_BIN = build/test_cst
EXAMPLE_SRCS = $(wildcard examples/*.c)
EXAMPLE_BINS = $(patsubst examples/%.c,build/%,$(EXAMPLE_SRCS))

.PHONY: all clean test examples demo

all: builddir $(LIB)

builddir:
	mkdir -p build

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: all
	$(CC) $(CFLAGS) -o $(TEST_BIN) $(TEST_SRC) -Lbuild -lcst_flood $(LDFLAGS)
	./$(TEST_BIN)

examples: all $(EXAMPLE_BINS)
	@echo "All examples built."

build/%: examples/%.c $(LIB)
	$(CC) $(CFLAGS) -o $@ $< -Lbuild -lcst_flood $(LDFLAGS)

demo: examples
	@echo "=== Running example1_tsi ==="
	./build/example1_tsi
	@echo ""
	@echo "=== Running example2_sosm ==="
	./build/example2_sosm
	@echo ""
	@echo "=== Running example3_boundary ==="
	./build/example3_boundary

clean:
	rm -rf build
