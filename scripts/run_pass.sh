#!/bin/bash

for f in `find $1 -name *.rbc`; do
    basename=$(basename $f)
    benchmark=${basename%.*.*}
    echo "===== Benchmark: $benchmark ====="
    mkdir $benchmark
    cp $f $benchmark/. -v
    outfile=$benchmark/${benchmark}_instr.bc
    echo "Saving result in $outfile"
    opt -S -mem2reg -loop-simplify -load DepGraph.so -load ComplexityInference.so -instnamer -LComp -instr-loop -mem2reg $f -o $outfile > ${benchmark}.eqs 2>&1
    clang $outfile
    ./a.out >> ../$benchmark.log
done
