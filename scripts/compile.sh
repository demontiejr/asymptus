#!/bin/bash

datasets=(MINI_DATASET SMALL_DATASET STANDARD_DATASET LARGE_DATASET EXTRALARGE_DATASET)

make -C $1 clean

for size in ${datasets[*]}; do
    echo "Dataset Size: $size"
    mkdir $size
    cd $size
    CFLAGS=-D$size make -C $1 -j8 CPPFLAGS+=-g OPTFLAGS="-O0" TEST=libcalls
    $HOME/parsing/run_pass.sh $1
    make -C $1 clean
    cd ..
done

$HOME/parsing/gen_results.sh
