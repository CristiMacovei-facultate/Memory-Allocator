#!/usr/bin/bash

make bd;

for file in $(ls tests); do
  echo "Running for $file...";

  cat "./tests/$file" | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./sfl
done;
