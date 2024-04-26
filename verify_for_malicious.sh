#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Wrong usage!"
    exit 1
fi

content=`cat $1`
DANGER_WORDS=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")

for word in "${DANGER_WORDS[@]}"; do
    if [[ $content == *"$word"* ]]; then
        echo $1
        exit 0
    fi
done

hasNonAscii=`grep -a -P '[^\x00-\x7F]' "$1" | sed '$d' | wc -l`

if [ -n hasNonAscii ]; then
    echo $1
    exit 0
fi

echo "SAFE"
exit 0