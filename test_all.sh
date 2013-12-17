#!/bin/sh

for f in dg_test/*.c;
do
  echo "processing $f file..."
  ./run2.sh $f 2>/dev/null
  filename=${f%.c}
  lli ${filename##*/}.splited.ll
  g++ $f -o $filename
  $filename
done
