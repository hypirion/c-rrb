#!/usr/bin/env bash

if [ "$(git rev-parse --abbrev-ref HEAD)" != "experimental" ]; then
    echo "Needs to run benchmarks in experimental mode:"
    echo "  Development and master branches only contain the most efficient options."
    exit 1
fi

# First ensure that configure is existing
if [ ! -x configure ]; then
    autoreconf --install
fi

export CC='clang'
export CFLAGS='-Ofast'

OPTIONS=(direct-append tail transients)
OPT_ACRO=('D' 'T' 'M')
OPT_PERMS=$(( 2 ** ${#OPTIONS[@]} ))
BRANCHING=5


WORDS="09 10
07 12 issues whoopsie 250000 200000 ninety seriously branching rrb wow coffee
Delete error edit branches merge fix Create JavaScript 6a Java 51 21 14"

function setup {
    local f="$1" # flags
    echo ./configure $f
    ./configure $f
    make clean
    make check
    if [ $? -ne 0 ]; then
        echo "Test run failed with following setup:"
        echo "CC='$CC' CFLAGS='$CFLAGS' ./configure $f && make clean && make check"
        exit $?
    fi
}

function benchmark {
    local DIR="$1" # flag acronym
    echo cd benchmark-suite
    cd benchmark-suite
    echo make benchmark
    make benchmark
    ./gen-all.sh "$DIR"
    cd ..
}


function bench_memory {
    local DIR="$1" # same dir all the time
    local BRANCHING="$2" # branching factor
    echo cd benchmark-suite
    cd benchmark-suite
    echo make benchmark
    make benchmark
    mkdir -p "$DIR"
    ./memory.sh "$DIR/rrb-memory-${BRANCHING}.dat"
    cd ..
}


for b in $BRANCHING; do
    for (( i=0; i < OPT_PERMS; i++ )); do
        FLAGS="--with-branching=$b"
        ACRO="rrb-$b-"
        for (( j=0; j < ${#OPTIONS[@]}; j++)); do
            if [ $(( (i >> j) & 1)) -eq 1 ]; then
                ENABLE_PRE="enable"
                ACRO="$ACRO${OPT_ACRO[$j]}"
            else
                ENABLE_PRE="disable"
            fi
            FLAGS="$FLAGS --$ENABLE_PRE-${OPTIONS[$j]}"
        done
        setup "$FLAGS"
        bench_memory "$ACRO" ""
        benchmark "$ACRO"
    done
done

function bench_rrb {
    local DIR="$1" # same dir all the time
    local BRANCHING="$2" # branching factor
    echo cd benchmark-suite
    cd benchmark-suite
    echo make benchmark
    make benchmark
    for cores in 4; do ## add in 1 2 8 for more cores
        for word in $WORDS; do
            echo ./bench-rrb.sh "$cores" "$word" data/sum-2013-01-10.json "$DIR" "$BRANCHING"
            ./bench-rrb.sh "$cores" "$word" data/sum-2013-01-10.json "$DIR" "$BRANCHING"
        done
    done
    cd ..
}

for b in 6 5 4 3 2; do
     FLAGS="--with-branching=$b --enable-rrb-debug --enable-direct-append --enable-tail --enable-transients"
     setup "$FLAGS"
     bench_rrb "branch" "$b"
     bench_memory "branch" "$b"
done
