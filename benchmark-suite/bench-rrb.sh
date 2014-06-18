#!/usr/bin/env bash

# like gen-tagged, but only running RRB benchmarks

CORES=$1
SEARCH="$2"
FILE="$3"
DIR="$4"
BRANCHING="$5"

mkdir -p "$PWD/$DIR"

RRB_OUTPUT="$PWD/$DIR/rrb-${CORES}-${SEARCH}-${BRANCHING}.dat"
RRB_PROGRAM="./pgrep_rrb"

ITERATIONS=50
WARMUP_RUNS=3

function remaining_runs {
    local rem=$(( $ITERATIONS ))
    if [ -f "$RRB_OUTPUT" ]; then
        rem=$(( $rem - `wc -l "$RRB_OUTPUT" | cut -f1 -d' '` ))
    fi
    echo $rem
}

function is_done {
    if [ -f "$1" ] && [ $(wc -l "$1" | cut -f1 -d' ') -ge $ITERATIONS ]; then
        return 0
    else
        return 1
    fi
}

function check_done {
    DONE=1
    if ! is_done "$RRB_OUTPUT"; then
        DONE=0
    fi
}

## Check if we've already finished this pass
check_done

## If we're not done
if [ $DONE -eq 0 ]; then

    echo "With ${CORES} cores, searching for '"${SEARCH}"'."

    ## Do a warmup
    for i in `seq 0 $(( $WARMUP_RUNS - 1 ))`; do
        echo -en "\rWarm-up: $i"
        $RRB_PROGRAM $CORES "$SEARCH" "$FILE" &>/dev/null
    done

    echo -e '\rWarm-up: Done'

    ## Then keep running until we're done
    while ! is_done "$RRB_OUTPUT"; do
        $RRB_PROGRAM $CORES "$SEARCH" "$FILE" 2>/dev/null >> "$RRB_OUTPUT"
        echo -en '\r'$(remaining_runs)' remaining runs. '
    done
    echo -e '\nDone!'
else
    echo 'Already finished -- skipping...'
fi

RRB_CANDLE="$PWD/$DIR/rrb-candle-${CORES}-${SEARCH}-${BRANCHING}.dat"

cd script

## This is cheap enough to repeat
./candlesticks.py "$RRB_OUTPUT" > "$RRB_CANDLE"

# if [ ! -f "calcs-$1-$2.pdf" ]; then
#     ./gen-calculations.sh "$DUMMY_CANDLE" "$ARRAY_CANDLE" "$RRB_CANDLE" "calcs-$1-$2.pdf"
#     echo script/gen-calculations.sh "$DUMMY_CANDLE" "$ARRAY_CANDLE" "$RRB_CANDLE" "script/calcs-$1-$2.pdf"
# fi
# if [ ! -f "cats-$1-$2.pdf" ]; then
#     ./gen-catenations.sh "$DUMMY_CANDLE" "$ARRAY_CANDLE" "$RRB_CANDLE" "cats-$1-$2.pdf"
#     echo script/gen-catenations.sh "$DUMMY_CANDLE" "$ARRAY_CANDLE" "$RRB_CANDLE" "script/cats-$1-$2.pdf"
# fi
cd ..
