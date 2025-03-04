#!/bin/bash

# "test_${i}.out" add this as a second parameter to lex.out to review output

i=1
for test_file in tests/phase1/*; do
    valgrind --leak-check=full ./lex.out "$test_file"  > "valgrind_${i}.out" 2>&1
    ((i++))
done

grep "definitely lost" valgrind_*.out

rm -v valgrind_*.out
