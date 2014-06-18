#!/usr/bin/env bash

SORTED=(whoopsie 250000 200000 ninety seriously branching rrb wow coffee Delete
        error edit branches merge fix Create JavaScript 6a Java 51 21 14 09 10
        07 12 issues)
SIZES=(4 33 49 104 161 334 730 1693 3579 14071 24746 43712 58513 64292 99908
       201848 297499 378365 468280 592902 700723 770307 878046 975886 1082471
       1208500 1519132 1565338)

# Convert uppercase to lowercase
LOOKUP="$(echo $2 | tr '[:upper:]' '[:lower:]')"
CORES="$1"
DIR="tmp"
if [ -n "$3" ]; then
    DIR="$3"
fi

declare -A hm
hm=([linesplit]=1 [linecat]=2 [searchfilter]=3 [searchcat]=4 [total]=5)
prettyprinted=("" "line split" "line concatenation" "search filtering"
               "search concatenation" "total")

function get_line {
    local in="$1"
    local line="$2"
    sed "${line}!d;q" "$in"
}

function total_cond {
    if [[ "$LOOKUP" == "total" ]]; then
        echo "$2"
    else
        echo "$1"
    fi
}

if [[ -z "${hm[$LOOKUP]}" ]]; then
    echo "gen-lined: Couldn't find the line number of ${LOOKUP}, quitting."
    exit 1
fi

LINE_NUM="${hm[$LOOKUP]}"

## Ensure all files are found
for b in 2 3 4 5 6; do
    for TYPE in rrb; do
        for TERM in ${SORTED[@]}; do
            FILE="$PWD/$DIR/$TYPE-candle-$CORES-$TERM-$b.dat"
            if [[ ! -r "$FILE" ]]; then
                echo "gen-lined: Couldn't find '${FILE}', needed for plot generation."
                echo "Quitting."
                exit 1
            fi
        done
    done
done

## Create aggregated data files
for b in 2 3 4 5 6; do
    for TYPE in rrb; do
        OUTPUT="$PWD/$DIR/$TYPE-total-$CORES-$b-$LOOKUP.dat"
        if [[ -f "$OUTPUT" ]]; then
            rm "$OUTPUT"
        fi
        for i in `seq 0 $(( ${#SORTED[@]} - 1 ))`; do
            TERM="${SORTED[$i]}"
            SIZE="${SIZES[$i]}"
            LINE=$(get_line "$PWD/$DIR/$TYPE-candle-$CORES-$TERM-$b.dat" "$LINE_NUM")
            echo "$SIZE $LINE \"$TERM\"" >> "$OUTPUT"
        done
    done
done
