<!--
SPDX-FileCopyrightText: 2018 Tuomas Siipola
SPDX-License-Identifier: MIT
-->

# jpp

[![Build Status](https://travis-ci.org/siiptuo/jpp.svg?branch=master)](https://travis-ci.org/siiptuo/jpp)

Pretty fast JSON pretty-printer.

## Features

- fast even with large inputs
- syntax highlighting
- encode Unicode escapes as UTF-8

## Usage

`jpp` reads JSON data from stdin and print-prints it to stdout.

You can for example pretty-print a JSON file:

    $ jpp < data.json

Or use `jpp` with other commands:

    $ curl -s https://api.myip.com | jpp

## Benchmark

Pretty-print
[countries.json](https://raw.githubusercontent.com/mledoze/countries/master/dist/countries.json)
(474K).

| Command               | Mean [ms]     | Min…Max [ms]  |
| --------------------- | ------------- | ------------- |
| `jpp`                 | 6.8 ± 1.1     | 5.9…14.8      |
| `json_reformat`       | 7.8 ± 0.8     | 7.3…13.3      |
| `jshon`               | 23.2 ± 1.4    | 22.0…32.0     |
| `jq`                  | 31.3 ± 2.7    | 29.1…45.1     |
| `json-glib-format`    | 41.7 ± 4.4    | 37.4…55.7     |
| `json_xs`             | 42.2 ± 3.9    | 39.0…59.7     |
| `aeson-pretty`        | 81.0 ± 8.3    | 75.1…111.2    |
| `python -m json.tool` | 125.0 ± 5.8   | 119.0…142.6   |
| `json_pp`             | 464.5 ± 12.8  | 446.7…484.3   |
| `prettier`            | 1686.8 ± 29.8 | 1630.8…1734.3 |

## License

MIT
