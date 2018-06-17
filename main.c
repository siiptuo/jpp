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
#include <unistd.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"

void parse_value();

int indent, colors;

void print_indent() {
  for (int i = 0; i < indent * 2; i++) putchar(' ');
}

void print_escape(char c) {
  if (colors) printf(COLOR_MAGENTA);
  putchar('\\');
  putchar(c);
  if (colors) printf(COLOR_RED);
}

char nextchar() {
  char c;
  while (isspace(c = getchar()));
  return c;
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
    default:
      if (colors) printf(COLOR_RESET);
      fprintf(stderr, "expected hex\n");
      exit(EXIT_FAILURE);
  }
}

void parse_unicode() {
  // Read Unicode code point.
  int c = readhex() << 12 | readhex() << 8 | readhex() << 4 | readhex();

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
    printf("\\u%04x", c);
    return;
  }

  // Otherwise print character using UTF-8.
  if (c < 0x80) {
    putchar(c);
  } else if (c < 0x0800) {
    putchar(0xc0 | (c >> 6));
    putchar(0x80 | (c & 0x3f));
  } else {
    putchar(0xe0 | (c >> 12));
    putchar(0x80 | ((c >> 6) & 0x3f));
    putchar(0x80 | (c & 0x3f));
  }
}

void parse_string() {
  if (colors) printf(COLOR_RED);
  while (1) {
    char c = getchar();
    if (iscntrl(c)) {
      if (colors) printf(COLOR_RESET);
      fprintf(stderr, "control character\n");
      exit(EXIT_FAILURE);
    }
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
          if (colors) printf(COLOR_RESET);
          fprintf(stderr, "unknown escape character: %c\n", c);
          exit(EXIT_FAILURE);
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
      fprintf(stderr, "expected sign or digit");
      exit(EXIT_FAILURE);
  }
  switch (c = getchar()) {
    case '0' ... '9':
      putchar(c);
      break;
    default:
      fprintf(stderr, "expected digit");
      exit(EXIT_FAILURE);
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
      if (colors) printf(COLOR_RESET);
      fprintf(stderr, "expected } or \"");
      exit(EXIT_FAILURE);
  }
  while (1) {
    parse_string();
    if (nextchar() != ':') {
      if (colors) printf(COLOR_RESET);
      fprintf(stderr, "expected :");
      exit(EXIT_FAILURE);
    }
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
        if (nextchar() != '"') {
          if (colors) printf(COLOR_RESET);
          fprintf(stderr, "expected \"");
          exit(EXIT_FAILURE);
        }
        putchar(',');
        putchar('\n');
        print_indent();
        putchar('"');
        continue;
      default:
        if (colors) printf(COLOR_RESET);
        fprintf(stderr, "expected } or ,");
        exit(EXIT_FAILURE);
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
        if (colors) printf(COLOR_RESET);
        fprintf(stderr, "expected ] or ,");
        exit(EXIT_FAILURE);
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
      putchar(c);
      putchar(getchar());
      putchar(getchar());
      putchar(getchar());
      if (colors) printf(COLOR_RESET);
      break;
    case 'f':
      if (colors) printf(COLOR_GREEN);
      putchar(c);
      putchar(getchar());
      putchar(getchar());
      putchar(getchar());
      putchar(getchar());
      if (colors) printf(COLOR_RESET);
      break;
    case 'n':
      if (colors) printf(COLOR_GREEN);
      putchar(c);
      putchar(getchar());
      putchar(getchar());
      putchar(getchar());
      if (colors) printf(COLOR_RESET);
      break;
    default:
      if (colors) printf(COLOR_RESET);
      fprintf(stderr, "unexpected character");
      exit(EXIT_FAILURE);
  }
}

int main() {
  colors = isatty(fileno(stdout));
  parse_value();
  if (nextchar() != EOF) {
    if (colors) printf(COLOR_RESET);
    fprintf(stderr, "expected EOF\n");
    exit(EXIT_FAILURE);
  }
  putchar('\n');
  return EXIT_SUCCESS;
}
