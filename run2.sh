#!/bin/bash
fpath=$1

filename=$(basename "$fpath")
fname="${filename%.*}"

echo "clang -emit-llvm -o $fname.bc -c $fpath"
clang -emit-llvm -o $fname.bc -c $fpath

echo "opt -indvars -instcombine -loop-simplify -simplifycfg -mem2reg $fname.bc > $fname.simple.bc"
opt -indvars -instcombine -loop-simplify -simplifycfg -mem2reg $fname.bc > $fname.simple.bc

echo "opt -load Release+Asserts/lib/splitpass.so -basicaa -da -dg -splitpass_scc < $fname.simple.bc > $fname.splited.bc "
opt -debug -load Release+Asserts/lib/splitpass.so -basicaa -da -dg -bp -splitpass2 < $fname.simple.bc > $fname.splited.bc 

test -e $fname.bc && rm $fname.bc
llvm-dis $fname.simple.bc
llvm-dis $fname.splited.bc
