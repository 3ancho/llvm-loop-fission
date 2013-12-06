#!/bin/bash
file=test2
clang -c -emit-llvm $file.c -o $file.bc
opt -mem2reg -simplifycfg -loop-simplify -instcombine -indvars $file.bc > $file.simple.bc
#opt -basicaa -da -load ../DG/Debug+Asserts/lib/DG.so -dg test.simple.bc >/dev/null
opt -basicaa -da -load ../Release+Asserts/lib/splitpass.so -dg $file.simple.bc >/dev/null 
