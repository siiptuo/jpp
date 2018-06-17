all: jpp

jpp: main.c
	@$(CC) -std=c99 -D_POSIX_C_SOURCE main.c -o jpp

test: jpp
	@./test.sh

clean:
	@rm -f jpp test-output.json

.PHONY: all test clean
