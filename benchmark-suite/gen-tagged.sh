#!/usr/bin/env bash

# like gen-plots, but with specified tmp folder.

CORES=$1
SEARCH="$2"
FILE="$3"
DIR="$4"

mkdir -p "$PWD/$DIR"

DUMMY_OUTPUT="$PWD/$DIR/overhead-$1-$2.dat"
ARRAY_OUTPUT="$PWD/$DIR/array-$1-$2.dat"
RRB_OUTPUT="$PWD/$DIR/rrb-$1-$2.dat"

OUTPUTS=("$DUMMY_OUTPUT" "$ARRAY_OUTPUT" "$RRB_OUTPUT")
PROGRAMS=('./pgrep_dummy' './pgrep_array' './pgrep_rrb')

ITERATIONS=50
WARMUP_RUNS=3

function remaining_runs {
    local rem=$(( $ITERATIONS * 3 ))
    for i in 0 1 2; do
        if [ -f "${OUTPUTS[$i]}" ]; then
            rem=$(( $rem - `wc -l "${OUTPUTS[$i]}" | cut -f1 -d' '` ))
        fi
    done
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
    for i in 0 1 2; do
        if ! is_done ${OUTPUTS[$i]}; then
            DONE=0
        fi
    done
}

## Check if we've already finished this pass
check_done

## If we're not done
if [ $DONE -eq 0 ]; then

    echo "With ${CORES} cores, searching for '"${SEARCH}"'."

    ## Do a warmup
    for i in `seq 0 $(( $WARMUP_RUNS - 1 ))`; do
        for j in 0 1 2; do
            echo -en '\rWarm-up: '$(( (3*$i + $j) * 100 / 9 ))'%'
            ${PROGRAMS[$i]} $CORES "$SEARCH" "$FILE" &>/dev/null
        done
    done

    echo -e '\rWarm-up: Done'

    ## Then keep running until we're done
    while [ $DONE -eq 0 ]; do
        for i in 0 1 2; do
            if ! is_done "${OUTPUTS[$i]}"; then
                ${PROGRAMS[$i]} $CORES "$SEARCH" "$FILE" 2>/dev/null >> "${OUTPUTS[$i]}"
            else
                ## Dummy-run for consistency
                ${PROGRAMS[$i]} $CORES "$SEARCH" "$FILE" &>/dev/null
            fi
            echo -en '\r'$(remaining_runs)' remaining runs. '
        done
        check_done
    done
    echo -e '\nDone!'
else
    echo 'Already finished -- skipping...'
fi

DUMMY_CANDLE="$PWD/$DIR/overhead-candle-$1-$2.dat"
ARRAY_CANDLE="$PWD/$DIR/array-candle-$1-$2.dat"
RRB_CANDLE="$PWD/$DIR/rrb-candle-$1-$2.dat"

cd script

## These are cheap enough to repeat
./candlesticks.py "$DUMMY_OUTPUT" > "$DUMMY_CANDLE"
./candlesticks.py "$ARRAY_OUTPUT" > "$ARRAY_CANDLE"
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
