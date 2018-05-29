.PHONY: all clean

# SANITIZE := -fuse-ld=gold -fsanitize=address -fsanitize=leak -fsanitize=undefined
# OPTIMIZE := -O0 -g
SANITIZE :=
OPTIMIZE := -O3

CC := gcc
CFLAGS := -std=gnu11 -O3 -DNDBUG
SRCS := $(shell find -name '*.c')
OBJS := $(addprefix build/,$(notdir $(SRCS:%.c=%.o)))

$(shell mkdir -p build)

all: build/libtds.a

test_pairingheap:
	$(CC) -std=gnu11 $(OPTIMIZE) $(SANITIZE) pairingheap.c -o build/pairingheap && build/pairingheap

test_aatree:
	$(CC) -std=gnu11 $(OPTIMIZE) $(SANITIZE) aatree.c -o build/aatree && build/aatree

test_bpt:
	$(CC) -std=gnu11 $(OPTIMIZE) $(SANITIZE) bpt.c -o build/bpt && build/bpt

build/libtds.a: $(OBJS)
	ar -rcs $@ $^

build/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf build

