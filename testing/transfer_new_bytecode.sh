#!/bin/bash

# remove old bytecode
make remove_bytecode

# compile the compiler
make remake

# generate new bytecode
make run_tests

# copy to vm
rm -vr ../alpha_vm/testing/byte_code/*
mkdir ../alpha_vm/testing/byte_code/b_3/ ../alpha_vm/testing/byte_code/b_4-5/
cp -vr testing/byte_code/b_3 ../alpha_vm/testing/byte_code/
cp -vr testing/byte_code/b_4-5 ../alpha_vm/testing/byte_code/
