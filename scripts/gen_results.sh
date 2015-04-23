#!/bin/bash

for f in `ls *.log`; do
  python ~/llvm/lib/Transforms/program-complexity-inference/scripts/gen_csv.py $f;
done

for p in `ls *.log`; do
  base=${p%.*};
  base=${base//-/_};
  for f in `ls ${base}*.csv`; do
    echo "=== File: $f";
    ../cpa $f;
  done > ${base}.out 2>&1;
done

for p in `ls *.log`; do
  base=${p%.*};
  f=${base//-/_}.out;
  eqs=SMALL_DATASET/${base}.eqs;
  echo "=== Benchmark: $base";
  python ~/llvm/lib/Transforms/program-complexity-inference/scripts/parse_complexity.py $eqs $f > ${base}-complexity;
done

rm *.out
#rm *.csv
