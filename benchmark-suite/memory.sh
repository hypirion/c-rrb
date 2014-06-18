#!/usr/bin/env bash

FILE="$1"

SORTED=(whoopsie 250000 200000 ninety seriously branching rrb wow coffee Delete
        error edit branches merge fix Create JavaScript 6a Java 51 21 14 09 10
        07 12 issues)
SIZES=(4 33 49 104 161 334 730 1693 3579 14071 24746 43712 58513 64292 99908
       201848 297499 378365 468280 592902 700723 770307 878046 975886 1082471
       1208500 1519132 1565338)

if [[ -f "$FILE" ]]; then
    echo "Presumably done with $FILE already, but redoing it."
    rm -f "$FILE"
fi

for i in `seq 0 1 $(( ${#SORTED[@]} - 1 ))`; do
    echo -n "${SIZES[$i]} " >> "$FILE"
    echo ./pgrep_mem_array 4 "${SORTED[$i]}" data/sum-2013-01-10.json to "$FILE"
    ./pgrep_mem_array 4 "${SORTED[$i]}" data/sum-2013-01-10.json 2>/dev/null 1>> "$FILE"
done
