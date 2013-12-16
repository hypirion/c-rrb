#!/usr/bin/env bash

SORTED=(whoopsie 250000 200000 ninety seriously branching rrb wow coffee Delete
        error edit branches merge fix Create JavaScript 6a Java 51 21 14 09 10
        07 12 issues)
SIZES=(4 33 49 104 161 334 730 1693 3579 14071 24746 43712 58513 73701 99908
       201848 297499 378365 468280 592902 700723 770307 878046 975886 1082471
       1208500 1519132 1565338)

CORES="$1"

mkdir -p "$PWD/tmp"

for TYPE in array rrb; do
    OUTPUT="$PWD/tmp/memory-${CORES}-${TYPE}.dat"
    if [[ ! -f "$OUTPUT" ]]; then
        echo "Please don't stop now, as this will bork up $OUTPUT."
        for i in `seq 0 $(( ${#SORTED[@]} - 1 ))`; do
            echo ./pgrep_mem_${TYPE} "$CORES" "${SORTED[$i]}" data/sum-2013-01-10.json
            res=$(./pgrep_mem_${TYPE} "$CORES" "${SORTED[$i]}" data/sum-2013-01-10.json 2>/dev/null)
            echo ${SIZES[$i]} $res >> "$OUTPUT"
        done
    fi
done
