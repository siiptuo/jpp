all: jpp

jpp: main.c
	@cc main.c -o jpp

test: jpp
	@./test.sh

clean:
	@rm -f jpp test-output.json

.PHONY: all test clean
