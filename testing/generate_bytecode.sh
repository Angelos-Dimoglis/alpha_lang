#!/bin/bash

for test_file in testing/tests/phase"$1"/*.asc; do
    filename=$(basename "$test_file" .asc)
    output_file="testing/byte_code/b_$1/${filename}.abc"
    
    echo "$test_file"
    ./alpha_compiler.out -i "$test_file" -b "$output_file" 1>/dev/null
done
