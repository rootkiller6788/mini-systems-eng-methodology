CC       = gcc
CFLAGS   = -std=c11 -Wall -Wextra -O2 -Iinclude -Wno-misleading-indentation
LDFLAGS  = -lm
AR       = ar

BUILDDIR = build
LIB      = $(BUILDDIR)/libhard_systems_methodology.a

SRCS     = $(wildcard src/*.c)
OBJS     = $(patsubst src/%.c,$(BUILDDIR)/%.o,$(SRCS))

TEST_SRC = $(wildcard tests/*.c)
TEST_BIN = $(patsubst tests/%.c,$(BUILDDIR)/test_%,$(TEST_SRC))

DEMO_SRC = $(wildcard examples/*.c)
DEMO_BIN = $(patsubst examples/%.c,$(BUILDDIR)/demo_%,$(DEMO_SRC))

.PHONY: all builddir clean test demo

all: builddir $(LIB)

builddir:
	@mkdir -p $(BUILDDIR)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

$(BUILDDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: builddir $(LIB) $(TEST_BIN)
	@echo "=== Running tests ==="
	@for t in $(TEST_BIN); do ./$$t || exit 1; done

$(BUILDDIR)/test_%: tests/%.c $(LIB)
	$(CC) $(CFLAGS) $< -L$(BUILDDIR) -lhard_systems_methodology $(LDFLAGS) -o $@

demo: builddir $(LIB) $(DEMO_BIN)
	@echo "=== Running demos ==="
	@for d in $(DEMO_BIN); do ./$$d || exit 1; done

$(BUILDDIR)/demo_%: examples/%.c $(LIB)
	$(CC) $(CFLAGS) $< -L$(BUILDDIR) -lhard_systems_methodology $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILDDIR)
