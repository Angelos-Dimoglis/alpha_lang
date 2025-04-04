#!/bin/bash

# "test_${i}.out" add this as a second parameter to lex.out to review output

for test_file in tests/phase1/*; do
    valgrind --leak-check=full ./lex.out "$test_file" 2>&1 | grep "definitely lost"
done
