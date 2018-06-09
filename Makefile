all: jpp

jpp: main.c
	@cc main.c -o jpp

test-output.json: jpp test-input.json
	@./jpp < test-input.json > test-output.json

test: test-expected.json test-output.json
	@diff test-expected.json test-output.json

clean:
	@rm -f jpp test-output.json

.PHONY: all test clean
