#!/bin/bash

datasets=(MINI_DATASET SMALL_DATASET STANDARD_DATASET LARGE_DATASET EXTRALARGE_DATASET)

for size in ${datasets[*]}; do
    echo "Dataset Size: $size"
    cd $size

    for benchmark in `ls -1d ./*/`; do
        echo "===== Benchmark: $benchmark ====="
        dir=$(basename $benchmark)
        cd $dir
        file=$dir.linked.rbc
        echo "File: $file"
        outfile=${dir}_instr.bc
        echo "Saving result in $outfile"
        opt -S -mem2reg -loop-simplify -load DepGraph.so -load ComplexityInference.so -instnamer -LComp -instr-loop -mem2reg $file -o $outfile > ${dir}.eqs 2>&1
        clang $outfile
        ./a.out >> ../../$dir.log
        cd ..
    done

    cd ..
done

$HOME/parsing/gen_results.sh
