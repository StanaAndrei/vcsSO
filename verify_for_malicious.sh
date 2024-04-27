#!/bin/bash

getOrd() {
    LC_CTYPE=C printf '%d' "'$1"
}

endWrong() {
    echo $1
    chmod 000 $1
    exit 0
}

if [ "$#" -ne 1 ]; then
    echo "Wrong usage!"
    exit 1
fi

chmod 777 $1
content=`cat $1`
DANGER_WORDS=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")
MIN_LINES=3
MAX_WORDS=1000
MAX_BYTES=2000

for word in "${DANGER_WORDS[@]}"; do
    if [[ $content == *"$word"* ]]; then
        endWrong $1
    fi
done

for i in $(seq 0 $((${#content} - 1))); do
    char="${content:i:1}"
    ord=$(getOrd "$char")
    if [ $ord -gt 127 ]; then
        endWrong $1
    fi
done

lines=$(wc -l < "$1")
bytes=$(wc -c < "$1")
words=$(wc -w < "$1")
if [ "$lines" -lt "$MIN_LINES" ] && [ "$words" -gt "$MAX_WORDS" ] && [ "$bytes" -gt "$MAX_BYTES" ]; then
   endWrong $1
fi

chmod 000 $1
echo "SAFE"
exit 0