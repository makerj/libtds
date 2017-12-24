.PHONY: all clean

CC := gcc
CFLAGS := -std=gnu11 -O3
SRCS := $(shell find -name '*.c')
OBJS := $(addprefix build/,$(notdir $(SRCS:%.c=%.o)))

$(shell mkdir -p build)

all: build/libtds.a

test_pairingheap:
	gcc -std=gnu11 -O3 -g pairingheap.c -o build/pairingheap && build/pairingheap

test_aatree:
	gcc -std=gnu11 -O3 -g aatree.c -o build/aatree && build/aatree

build/libtds.a: $(OBJS)
	ar -rcs $@ $^

build/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf build

