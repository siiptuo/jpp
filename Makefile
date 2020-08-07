# SPDX-FileCopyrightText: 2018 Tuomas Siipola
# SPDX-License-Identifier: CC0-1.0

all: jpp

jpp: main.c
	$(CC) -std=c99 -D_POSIX_C_SOURCE -Werror -Wall -Wextra -Wpedantic -O2 main.c -o jpp

test: jpp
	@./test.sh

clean:
	@rm -f jpp test-output.json

.PHONY: all test clean
