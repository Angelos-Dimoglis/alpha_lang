#!/bin/sh

if [ ! -f alpha_compiler.out ]; then
    echo "alpha compiler doesnt exist"
    return
fi

for test_file in testing/tests/phase3/*; do
    echo
    echo "test: $test_file"
    ./alpha_compiler.out "$test_file" 1>/dev/null
done
