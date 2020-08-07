#!/usr/bin/env bash

./jpp < test-input.json > test-output.json
diff test-expected.json test-output.json &> /dev/null
if [ $? -eq 0 ]; then
  echo "OK   test-input.json"
else
  echo "FAIL test-input.json"
fi

for f in jsonchecker/pass*; do
  ./jpp < $f &> /dev/null
  if [ $? -eq 0 ]; then
    echo "OK   $f"
  else
    echo "FAIL $f"
  fi
done

# Skip fail1.json because top-level value can be anything, not just object or array
for f in jsonchecker/fail{2..33}.json; do
  ./jpp < $f &> /dev/null
  if [ $? -eq 0 ]; then
    echo "FAIL $f"
  else
    echo "OK   $f"
  fi
done

for f in jsontestsuite/y_*; do
  ./jpp < $f &> /dev/null
  if [ $? -eq 0 ]; then
    echo "OK   $f"
  else
    echo "FAIL $f"
  fi
done

for f in jsontestsuite/n_*; do
  ./jpp < $f &> /dev/null
  if [ $? -eq 0 ]; then
    echo "FAIL $f"
  else
    echo "OK   $f"
  fi
done
