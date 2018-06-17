#!/usr/bin/env bash

./jpp < test-input.json > test-output.json
diff test-expected.json test-output.json &> /dev/null
if [ $? -eq 0 ]; then
  echo "OK   test-input.json"
else
  echo "FAIL test-input.json"
fi

for f in test/pass*; do
  ./jpp < $f &> /dev/null
  if [ $? -eq 0 ]; then
    echo "OK   $f"
  else
    echo "FAIL $f"
  fi
done

# skip fail1.json and fail18.json
for f in test/fail{2..17}.json test/fail{19..33}.json; do
  ./jpp < $f &> /dev/null
  if [ $? -eq 0 ]; then
    echo "FAIL $f"
  else
    echo "OK   $f"
  fi
done
