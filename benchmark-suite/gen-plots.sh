#!/usr/bin/env bash

CORES=$1
SEARCH="$2"
FILE="$3"

DUMMY_OUTPUT="$(mktemp /tmp/dummy-XXXXXXXXXXXXX)"
ARRAY_OUTPUT="$(mktemp /tmp/array-XXXXXXXXXXXXX)"
RRB_OUTPUT="$(mktemp /tmp/rrb-XXXXXXXXXXXXX)"

ITERATIONS=100

for i in `seq 0 $(( $ITERATIONS - 1 ))`; do
    echo -en '\r'$(( (3*$i + 0) * 100 / (3 * $ITERATIONS) ))'%'
    ./pgrep_dummy $CORES "$SEARCH" "$FILE" 2>/dev/null >> "$DUMMY_OUTPUT"
    echo -en '\r'$(( (3*$i + 1) * 100 / (3 * $ITERATIONS) ))'%'
    ./pgrep_array $CORES "$SEARCH" "$FILE" 2>/dev/null >> "$ARRAY_OUTPUT"
    echo -en '\r'$(( (3*$i + 2) * 100 / (3 * $ITERATIONS) ))'%'
    ./pgrep_rrb   $CORES "$SEARCH" "$FILE" 2>/dev/null >> "$RRB_OUTPUT"
done

echo -e '\r100%'

DUMMY_CANDLE="$(mktemp /tmp/dummy-XXXXXXXXXXXXX)"
ARRAY_CANDLE="$(mktemp /tmp/array-XXXXXXXXXXXXX)"
RRB_CANDLE="$(mktemp /tmp/rrb-XXXXXXXXXXXXX)"

cd script
./candlesticks.py "$DUMMY_OUTPUT" > "$DUMMY_CANDLE"
./candlesticks.py "$ARRAY_OUTPUT" > "$ARRAY_CANDLE"
./candlesticks.py "$RRB_OUTPUT" > "$RRB_CANDLE"

echo script/gen-calculations.sh "$DUMMY_CANDLE" "$ARRAY_CANDLE" "$RRB_CANDLE" 'script/calcs.pdf'
./gen-calculations.sh "$DUMMY_CANDLE" "$ARRAY_CANDLE" "$RRB_CANDLE" 'calcs.pdf'
echo script/gen-catenations.sh "$DUMMY_CANDLE" "$ARRAY_CANDLE" "$RRB_CANDLE" 'script/cats.pdf'
./gen-catenations.sh "$DUMMY_CANDLE" "$ARRAY_CANDLE" "$RRB_CANDLE" 'cats.pdf'
cd ..

# Cleanup
echo "Candlestick output lies within $DUMMY_CANDLE,
$ARRAY_CANDLE and $RRB_CANDLE."
rm $DUMMY_OUTPUT $ARRAY_OUTPUT $RRB_OUTPUT
