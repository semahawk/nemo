#!/bin/sh

# A small shell script to launch all the test files.
# Note, it's got to be ran directly from the root directory (not 't' or anything).

if [ ! -f ./nemo ]; then
  echo "Nemo seems to be not compiled. Run \`make\`."
  exit 1
fi

fail=0

for i in t/*.nm; do
  echo "Running tests in \"`basename $i`\""
  eval ./nemo $i
  if [ $? -ne 0 ]; then
    fail=1
  fi
done

echo -n "Done"

if [ $fail -eq 1 ]; then
  echo ", but something went wrong."
  exit 1
else
  echo "!"
fi
