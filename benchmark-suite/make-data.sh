#!/usr/bin/env bash

echo http://data.githubarchive.org/2013-01-{01..10}-{0..23}.json.gz \
    | xargs -n 1 -P 32 wget -q -nc

for i in $(find . -name '*.json.gz'); do
    gunzip $i;
done

cat 2013-01-*.json > data/sum-2013-01-10.json
rm 2013-01-*.json
