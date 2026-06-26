CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wno-misleading-indentation -O2 -Iinclude
LDFLAGS = -lm
AR = ar
LIB = build/libsoft_systems.a

SRCS = src/ssm_core.c src/ssm_rootdef.c src/ssm_conceptual.c src/ssm_comparison.c src/ssm_streams.c src/ssm_intervention.c src/ssm_eval.c

OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

TEST_SRC = tests/test_ssm.c
TEST_BIN = build/test_ssm

EXAMPLE_SRCS = $(wildcard examples/*.c)
EXAMPLE_BINS = $(patsubst examples/%.c,build/%,$(EXAMPLE_SRCS))

.PHONY: all builddir clean test examples demo

all: builddir $(LIB)

builddir:
	mkdir -p build

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: all
	$(CC) $(CFLAGS) -o $(TEST_BIN) $(TEST_SRC) -Lbuild -lsoft_systems $(LDFLAGS)
	./$(TEST_BIN)

examples: all $(EXAMPLE_BINS)
	@echo "All examples built."

build/%: examples/%.c $(LIB)
	$(CC) $(CFLAGS) -o $@ $< -Lbuild -lsoft_systems $(LDFLAGS)

demo: examples
	@echo "Running examples..."
	for f in $(EXAMPLE_BINS); do echo "=== $$f ==="; ./$$f; echo; done

clean:
	rm -rf build
