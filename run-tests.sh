#!/usr/bin/env bash

# First ensure that configure is existing
if [ ! -x configure ]; then
    autoreconf --install
fi

export CC='clang'
export CFLAGS='-O0 -g'

OPTIONS=(rrb-debug direct-append tail)
OPT_PERMS=$(( 2 ** ${#OPTIONS[@]} ))

if [ "$1" = "full" ]; then
    BRANCHING=`seq 2 5`
else
    BRANCHING=5
fi

for b in $BRANCHING; do 
    for (( i=0; i < OPT_PERMS; i++ )); do
        FLAGS="--with-branching=$b"
        for (( j=0; j < ${#OPTIONS[@]}; j++)); do
            if [ $(( (i >> j) & 1)) -eq 1 ]; then
                ENABLE_PRE="enable"
            else
                ENABLE_PRE="disable"
            fi
            FLAGS="$FLAGS --$ENABLE_PRE-${OPTIONS[$j]}"
        done
        echo ./configure $FLAGS
        ./configure $FLAGS
        make clean
        make check
        if [ $? -ne 0 ]; then
            echo "Test run failed with following setup:"
            echo "CC='$CC' CFLAGS='$CFLAGS' ./configure $FLAGS && make clean && make check"
            exit $?
        fi
    done
done
