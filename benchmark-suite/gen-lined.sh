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
if [ -n "$1" ]; then
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
for TYPE in array rrb overhead; do
    for TERM in ${SORTED[@]}; do
        FILE="$PWD/$DIR/$TYPE-candle-$CORES-$TERM.dat"
        if [[ ! -r "$FILE" ]]; then
            echo "gen-lined: Couldn't find '${FILE}', needed for plot generation."
            echo "Quitting."
            exit 1
        fi
    done
done

## Create aggregated data files
for TYPE in array rrb overhead; do
    OUTPUT="$PWD/$DIR/$TYPE-total-$CORES-$LOOKUP.dat"
    if [[ -f "$OUTPUT" ]]; then
        rm "$OUTPUT"
    fi
    for i in `seq 0 $(( ${#SORTED[@]} - 1 ))`; do
        TERM="${SORTED[$i]}"
        SIZE="${SIZES[$i]}"
        LINE=$(get_line "$PWD/$DIR/$TYPE-candle-$CORES-$TERM.dat" "$LINE_NUM")
        echo "$SIZE $LINE \"$TERM\"" >> "$OUTPUT"
    done
done

mkdir -p script

FT="\\footnotesize"

gnuplot <<EOF
set terminal pdf mono font 'Bitstream Charter'
set termoption dash

set output "$PWD/plots/total-$CORES-$LOOKUP.pdf"
dummy="$PWD/$DIR/overhead-total-$CORES-$LOOKUP.dat"
array="$PWD/$DIR/array-total-$CORES-$LOOKUP.dat"
rrb="$PWD/$DIR/rrb-total-$CORES-$LOOKUP.dat"

set title 'Average time used $(total_cond 'for the' 'in') ${prettyprinted[$LINE_NUM]}$(total_cond ' phase' '').'
set key inside left top box linetype -1 linewidth 1.000
#set key inside right bottom box linetype -1 linewidth 1.000 width 6

set xtic auto
set ytic auto

set xlabel 'Lines matching (in millions)'
set ylabel 'Milliseconds'
# set format y '%.0f'
# set format x '%.0f'

plot dummy u (\$1/1000000):(\$4/1000000) with linespoints lt 1 lc rgb 'blue' pt 1 t 'Overhead',\
     array u (\$1/1000000):(\$8/1000000):(\$3/1000000):(\$5/1000000) with yerrorlines lt 1 lc rgb 'web-green' pt 2 t 'Array',\
     rrb u (\$1/1000000):(\$4/1000000) with linespoints lt 1 lc rgb 'dark-red' pt 8 t 'RRB',\
     x*13 t '$\\O(n)$' lt 1 lc rgb 'dark-gray'

set terminal epslatex
set output "$PWD/script/total-$CORES-$LOOKUP.tex"
replot
EOF
