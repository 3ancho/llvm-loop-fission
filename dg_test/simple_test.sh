#!/bin/bash
file=loop1
input=10
clang -c -emit-llvm $file.c -o $file.bc
opt -indvars -instcombine -loop-simplify -simplifycfg -mem2reg $file.bc > $file.simple.bc
#opt -load ../Release+Asserts/lib/splitpass.so -basicaa -da -dg -scc --debug-pass=Structure < $file.simple.bc > /dev/null
opt -load ../Debug+Asserts/lib/splitpass.so -basicaa -da -dg -scc --debug-pass=Structure < $file.simple.bc > /dev/null
llvm-dis $file.simple.bc
llc $file.simple.bc -o $file.s
g++ -o $file $file.s
./$file $input
