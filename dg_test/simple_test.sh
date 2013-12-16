#!/bin/bash
file=$1
input=$2
clang -c -emit-llvm $file.c -o $file.bc
opt -indvars -instcombine -loop-simplify -simplifycfg -mem2reg $file.bc > $file.simple.bc
#opt $file.bc > $file.simple.bc
#opt -load ../Release+Asserts/lib/splitpass.so -basicaa -da -dg -scc --debug-pass=Structure < $file.simple.bc > /dev/null
opt -load ../Debug+Asserts/lib/splitpass.so -basicaa -da -dg -bp --debug-pass=Structure < $file.simple.bc
llvm-dis $file.simple.bc
llc $file.simple.bc -o $file.s
g++ -o $file $file.s
./$file $input
