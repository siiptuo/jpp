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

#define DIGIT '0': case '1': case '2': case '3': case '4': case '5': \
                   case '6': case '7': case '8': case '9'

void parse_value();

int indent;

void print_indent() {
  for (int i = 0; i < indent * 2; i++) putchar(' ');
}

char nextchar() {
  char c;
  while (isspace(c = getchar()));
  return c;
}

void parse_string() {
  while (putchar(getchar()) != '"');
}

void parse_exponent() {
  char c = getchar();
  switch (c) {
    case '+':
      break;
    case '-':
      putchar('-');
      break;
    case DIGIT:
      putchar(c);
      break;
    default:
      fprintf(stderr, "expected sign or digit");
      exit(EXIT_FAILURE);
  }
  while (1) {
    switch (c = getchar()) {
      case DIGIT:
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
      case DIGIT:
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
      case DIGIT:
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

void parse_object() {
  int c = nextchar();
  if (c == '}') {
    indent--;
    putchar('}');
    return;
  }
  ungetc(c, stdin);
  putchar('\n');
  print_indent();
  while (1) {
    switch (nextchar()) {
      case '}':
        putchar('\n');
        indent--;
        print_indent();
        putchar('}');
        return;
      case ',':
        putchar(',');
        putchar('\n');
        print_indent();
        continue;
      case '"':
        putchar('"');
        parse_string();
        if (nextchar() != ':') {
          fprintf(stderr, "expected :");
          exit(EXIT_FAILURE);
        }
        putchar(':');
        putchar(' ');
        parse_value();
        break;
      default:
        fprintf(stderr, "expected \" or ,");
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
        fprintf(stderr, "expected ] or ,");
        exit(EXIT_FAILURE);
    }
  }
}

void parse_value() {
  switch (putchar(nextchar())) {
    case '"':
      parse_string();
      break;
    case '-':
    case DIGIT:
      parse_number();
      break;
    case '{':
      indent++;
      parse_object();
      break;
    case '[':
      indent++;
      parse_array();
      break;
    case 't':
      putchar(getchar());
      putchar(getchar());
      putchar(getchar());
      break;
    case 'f':
      putchar(getchar());
      putchar(getchar());
      putchar(getchar());
      putchar(getchar());
      break;
    case 'n':
      putchar(getchar());
      putchar(getchar());
      putchar(getchar());
      break;
    default:
      exit(EXIT_FAILURE);
  }
}

int main() {
  parse_value();
  putchar('\n');
  return EXIT_SUCCESS;
}
