#!/bin/bash

for test_file in testing/tests/phase3/*.asc; do
    filename=$(basename "$test_file" .asc)
    output_file="testing/byte_code/${filename}.abc"
    
    echo "./alpha_compiler.out -i \"$test_file\" -b \"$output_file\""
    ./alpha_compiler.out -i "$test_file" -b "$output_file" 1>/dev/null
done

