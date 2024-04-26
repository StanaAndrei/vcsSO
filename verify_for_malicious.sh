#!/bin/bash


if [ "$#" -ne 1 ]; then
    echo "Wrong usage!"
    exit 1
fi

chmod 777 $1
content=`cat $1`
DANGER_WORDS=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")

for word in "${DANGER_WORDS[@]}"; do
    if [[ $content == *"$word"* ]]; then
        echo $1
        exit 0
    fi
done

getOrd() {
    LC_CTYPE=C printf '%d' "'$1"
}

for i in $(seq 0 $((${#content} - 1))); do
    char="${content:i:1}"
    ord=$(getOrd "$char")
    if [ $ord -gt 127 ]; then
        echo $1
        exit 0
    fi
done

chmod 000 $1
echo "SAFE"
exit 0