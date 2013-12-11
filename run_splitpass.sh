#!/bin/bash
fpath=$1

filename=$(basename "$fpath")
fname="${filename%.*}"

echo "clang -emit-llvm -o $fname.bc -c $fpath"
clang -emit-llvm -o $fname.bc -c $fpath

echo "opt -mem2reg -simplifycfg -loop-simplify -instcombine -indvars $fname.bc > $fname.simple.bc"
opt -mem2reg -simplifycfg -loop-simplify -instcombine -indvars $fname.bc > $fname.simple.bc

echo "opt -load ~/splitpass/Release+Asserts/lib/splitpass.so -splitpass < $fname.simple.bc > $fname.splited.bc"
opt -load ~/splitpass/Release+Asserts/lib/splitpass.so -splitpass < $fname.simple.bc > $fname.splited.bc

echo "llvm-dis $fname.splited.bc"
llvm-dis $fname.splited.bc

test -e $fname.bc && rm $fname.bc
test -e $fname.simple.bc && rm $fname.simple.bc
test -e $fname.splited.bc && rm $fname.splited.bc

