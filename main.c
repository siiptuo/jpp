/*
 * Copyright (c) 2018 Tuomas Siipola
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"

void parse_value();

int indent, colors;

#define fail(...) \
  do { \
    if (colors) printf(COLOR_RESET); \
    fprintf(stderr, __VA_ARGS__); \
    exit(EXIT_FAILURE); \
  } while (0) \

#define INDENT_BUFFER_SIZE 255

char indent_buffer[INDENT_BUFFER_SIZE] = {[0 ... INDENT_BUFFER_SIZE - 1] = ' '};

void print_indent() {
  size_t i = 2 * indent;
  for (; i > INDENT_BUFFER_SIZE; i -= INDENT_BUFFER_SIZE)
    fwrite(indent_buffer, 1, INDENT_BUFFER_SIZE, stdout);
  fwrite(indent_buffer, 1, i, stdout);
}

void print_escape(char c) {
  if (colors) printf(COLOR_MAGENTA);
  putchar('\\');
  putchar(c);
  if (colors) printf(COLOR_RED);
}

int nextchar() {
  int c;
  while (isspace(c = getchar()));
  return c;
}

void expect(int c) {
  if (getchar() != c) fail("expected %c", c);
  putchar(c);
}

int readhex() {
  switch (getchar()) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default: fail("expected hex\n");
  }
}

void print_utf8(uint32_t c) {
  if (c < 0x80) {
    putchar(c);
  } else if (c < 0x0800) {
    putchar(0xc0 | (c >> 6));
    putchar(0x80 | (c & 0x3f));
  } else if (c < 0x10000) {
    putchar(0xe0 | (c >> 12));
    putchar(0x80 | ((c >> 6) & 0x3f));
    putchar(0x80 | (c & 0x3f));
  } else {
    putchar(0xf0 | (c >> 18));
    putchar(0x80 | ((c >> 12) & 0x3f));
    putchar(0x80 | ((c >> 6) & 0x3f));
    putchar(0x80 | (c & 0x3f));
  }
}

void parse_unicode() {
  // Read Unicode code point.
  uint16_t c = readhex() << 12 | readhex() << 8 | readhex() << 4 | readhex();

  // Handle UTF-16 surrogate pairs. Check for high surrogate.
  if (c >> 10 == 0x36) {
    // Read low surrogate.
    if (getchar() != '\\') fail("expected \\");
    if (getchar() != 'u') fail("expected u");
    uint16_t d = readhex() << 12 | readhex() << 8 | readhex() << 4 | readhex();
    if (d >> 10 != 0x37) fail("expected low surrogate");
    print_utf8(0x10000 | (c & 0x3ff) << 10 | d & 0x3ff);
    return;
  }

  // Print character escape sequence if it has one.
  switch (c) {
    case '"':
    case '\\':
    case '/':
      print_escape(c);
      return;
    case '\b': print_escape('b'); return;
    case '\f': print_escape('f'); return;
    case '\n': print_escape('n'); return;
    case '\r': print_escape('r'); return;
    case '\t': print_escape('t'); return;
    default:
      break;
  }

  // Control characters are not allowed in JSON strings.
  if (iscntrl(c)) {
    if (colors) printf(COLOR_MAGENTA);
    printf("\\u%04x", c);
    if (colors) printf(COLOR_RED);
    return;
  }

  print_utf8(c);
}

void parse_string() {
  if (colors) printf(COLOR_RED);
  while (1) {
    char c = getchar();
    if (iscntrl(c)) fail("control character\n");
    if (c == '"') {
      if (colors) printf(COLOR_RESET);
      putchar(c);
      return;
    } else if (c == '\\') {
      switch (c = getchar()) {
        case '"':
        case '\\':
        case '/':
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't':
          print_escape(c);
          break;
        case 'u':
          parse_unicode();
          break;
        default:
          fail("unknown escape character: %c\n", c);
      }
    } else {
      putchar(c);
    }
  }
}

void parse_exponent() {
  char c = getchar();
  switch (c) {
    case '+':
      break;
    case '-':
      putchar('-');
      break;
    case '0' ... '9':
      putchar(c);
      goto rest;
    default:
      fail("expected sign or digit");
  }
  switch (c = getchar()) {
    case '0' ... '9':
      putchar(c);
      break;
    default:
      fail("expected digit");
  }
rest:
  while (1) {
    switch (c = getchar()) {
      case '0' ... '9':
        putchar(c);
        break;
      default:
        ungetc(c, stdin);
        return;
    }
  }
}

void parse_decimal() {
  while (1) {
    char c = getchar();
    switch (c) {
      case '0' ... '9':
        putchar(c);
        break;
      case 'e':
      case 'E':
        putchar('e');
        parse_exponent();
        break;
      default:
        ungetc(c, stdin);
        return;
    }
  }
}

void parse_number() {
  while (1) {
    char c = getchar();
    switch (c) {
      case '0' ... '9':
        putchar(c);
        break;
      case 'e':
      case 'E':
        putchar('e');
        parse_exponent();
        break;
      case '.':
        putchar('.');
        parse_decimal();
        break;
      default:
        ungetc(c, stdin);
        return;
    }
  }
}

void parse_zero() {
  char c = getchar();
  switch (c) {
    case 'e':
    case 'E':
      putchar('e');
      parse_exponent();
      break;
    case '.':
      putchar('.');
      parse_decimal();
      break;
    default:
      ungetc(c, stdin);
      break;
  }
}

void parse_negative_number() {
  char c = getchar();
  switch (c) {
    case '0':
      putchar(c);
      parse_zero();
      break;
    case '1' ... '9':
      putchar(c);
      parse_number();
      break;
    default:
      ungetc(c, stdin);
      break;
  }
}

void parse_object() {
  switch (nextchar()) {
    case '}':
      indent--;
      putchar('}');
      return;
    case '"':
      putchar('\n');
      print_indent();
      putchar('"');
      break;
    default:
      fail("expected } or \"");
  }
  while (1) {
    parse_string();
    if (nextchar() != ':') fail("expected :");
    putchar(':');
    putchar(' ');
    parse_value();
    switch (nextchar()) {
      case '}':
        putchar('\n');
        indent--;
        print_indent();
        putchar('}');
        return;
      case ',':
        putchar(',');
        if (nextchar() != '"') fail("expected \"");
        putchar('\n');
        print_indent();
        putchar('"');
        continue;
      default:
        fail("expected } or ,");
    }
  }
}

void parse_array() {
  int c = nextchar();
  if (c == ']') {
    indent--;
    putchar(']');
    return;
  }
  ungetc(c, stdin);
  putchar('\n');
  while (1) {
    print_indent();
    parse_value();
    switch (nextchar()) {
      case ']':
        putchar('\n');
        indent--;
        print_indent();
        putchar(']');
        return;
      case ',':
        putchar(',');
        putchar('\n');
        continue;
      default:
        fail("expected ] or ,");
    }
  }
}

void parse_value() {
  char c = nextchar();
  switch (c) {
    case '"':
      putchar(c);
      parse_string();
      break;
    case '-':
      if (colors) printf(COLOR_GREEN);
      putchar(c);
      parse_negative_number();
      if (colors) printf(COLOR_RESET);
      break;
    case '0':
      if (colors) printf(COLOR_GREEN);
      putchar(c);
      parse_zero();
      if (colors) printf(COLOR_RESET);
      break;
    case '1' ... '9':
      if (colors) printf(COLOR_GREEN);
      putchar(c);
      parse_number();
      if (colors) printf(COLOR_RESET);
      break;
    case '{':
      putchar(c);
      indent++;
      parse_object();
      break;
    case '[':
      putchar(c);
      indent++;
      parse_array();
      break;
    case 't':
      if (colors) printf(COLOR_GREEN);
      putchar('t');
      expect('r');
      expect('u');
      expect('e');
      if (colors) printf(COLOR_RESET);
      break;
    case 'f':
      if (colors) printf(COLOR_GREEN);
      putchar('f');
      expect('a');
      expect('l');
      expect('s');
      expect('e');
      if (colors) printf(COLOR_RESET);
      break;
    case 'n':
      if (colors) printf(COLOR_GREEN);
      putchar('n');
      expect('u');
      expect('l');
      expect('l');
      if (colors) printf(COLOR_RESET);
      break;
    default:
      if (colors) printf(COLOR_RESET);
      fail("unexpected character");
      exit(EXIT_FAILURE);
  }
}

int main() {
  // `stdin` is line buffered by default. This means that it will be flushed
  // frequently with already formatted input. To prevent this, let's make it
  // block buffered.
  char buf[BUFSIZ];
  setbuf(stdin, buf);

  // Enable colors if output is TTY.
  colors = isatty(fileno(stdout));

  parse_value();
  if (nextchar() != EOF) fail("expected EOF");
  putchar('\n');

  return EXIT_SUCCESS;
}
