#!/usr/bin/env bash

SORTED=(whoopsie 250000 200000 ninety seriously branching rrb wow coffee Delete
        error edit branches merge fix Create JavaScript 6a Java 51 21 14 09 10
        07 12 issues)
SIZES=(4 33 49 104 161 334 730 1693 3579 14071 24746 43712 58513 64292 99908
       201848 297499 378365 468280 592902 700723 770307 878046 975886 1082471
       1208500 1519132 1565338)

# Convert uppercase to lowercase
CORES="$1"

## Ensure all files are found
for TYPE in array rrb; do
    FILE="$PWD/tmp/memory-$CORES-$TYPE.dat"
    if [[ ! -r "$FILE" ]]; then
        echo "memory-plots.sh: Couldn't find '${FILE}', needed for plot generation."
        echo "Quitting."
        echo "(The file should be possible to generate trough gen-memory.sh)"
        exit 1
    fi
done

## Create aggregated data files

mkdir -p script

gnuplot <<EOF
set terminal pdf mono font 'Bitstream Charter'
set termoption dash

set output "$PWD/plots/memory-search-$CORES.pdf"
array="$PWD/tmp/memory-$CORES-array.dat"
rrb="$PWD/tmp/memory-$CORES-rrb.dat"

set title 'Memory usage at end of search phase.'
set key inside left top box linetype -1 linewidth 1.000
#set key inside right bottom box linetype -1 linewidth 1.000 width 6

set xtic auto
set ytic auto

set xlabel 'Lines matching (in millions)'
set ylabel 'Megabytes (\$1000^2\$ bytes)'
# set format y '%.0f'
# set format x '%.0f'

plot 8*x t 'No overhead' lt 1 lc rgb 'blue',\
     array u (\$1/1000000 <= 0.20 ? 1/0 : \$1/1000000):(\$2/1000000) with linespoints lt 1 lc rgb 'web-green' pt 2 t 'Array',\
     array u (\$1/1000000):(\$2/1000000) with lines lt 1 lc rgb 'web-green' t '',\
     rrb u (\$1/1000000 <= 0.20 ? 1/0 : \$1/1000000):(\$2/1000000) with linespoints lt 1 lc rgb 'dark-red' pt 8 t 'RRB',\
     rrb u (\$1/1000000):(\$2/1000000) with lines lt 1 lc rgb 'dark-red' t ''

set terminal epslatex
set output "$PWD/script/memory-search-$CORES.tex"
replot
EOF

gnuplot <<EOF
set terminal pdf mono font 'Bitstream Charter'
set termoption dash

set output "$PWD/plots/memory-concat-$CORES.pdf"
array="$PWD/tmp/memory-$CORES-array.dat"
rrb="$PWD/tmp/memory-$CORES-rrb.dat"

set title 'Memory usage for most expensive concatenation.'
set key inside left top box linetype -1 linewidth 1.000
#set key inside right bottom box linetype -1 linewidth 1.000 width 6

set xtic auto
set ytic auto

set xlabel 'Lines matching (in millions)'
set ylabel 'Megabytes (\$1000^2\$ bytes)'
# set format y '%.0f'
# set format x '%.0f'

plot 8*x t 'No overhead' lt 1 lc rgb 'blue',\
     array u (\$1/1000000 <= 0.20 ? 1/0 : \$1/1000000):(\$3/1000000) with linespoints lt 1 lc rgb 'web-green' pt 2 t 'Array',\
     array u (\$1/1000000):(\$3/1000000) with lines lt 1 lc rgb 'web-green' t '',\
     rrb u (\$1/1000000 <= 0.20 ? 1/0 : \$1/1000000):(\$3/1000000) with linespoints lt 1 lc rgb 'dark-red' pt 8 t 'RRB',\
     rrb u (\$1/1000000):(\$3/1000000) with lines lt 1 lc rgb 'dark-red' t ''

set terminal epslatex
set output "$PWD/script/memory-concat-$CORES.tex"
replot
EOF
