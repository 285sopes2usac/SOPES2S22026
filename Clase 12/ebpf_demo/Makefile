ARCH_INC := /usr/include/$(shell uname -m)-linux-gnu

all: openat_watch openat_watch.bpf.o

openat_watch.bpf.o: openat_watch.bpf.c openat_watch.h
	clang -O2 -g -target bpf -I$(ARCH_INC) -c $< -o $@

openat_watch: openat_watch.c openat_watch.h
	gcc -O2 -Wall -Wextra -o $@ $< -lbpf -lelf -lz

clean:
	rm -f openat_watch openat_watch.bpf.o
