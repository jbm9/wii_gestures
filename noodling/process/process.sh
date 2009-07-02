#!/usr/bin/env bash
echo 'data = []; meta = []' > data.py
for i in ../training-data-capture-20090610/*; do 
    awk -v file="${i##*/}" -v cnt=0 -F '[=, ]' '/Acc/ { split(file, arr, "\."); printf "data += [[%s, %s, %s]]\nmeta += [(\"%s\", \"%s\", %s )]\n", $4, $7, $10, arr[1], arr[2], cnt  } /Button Report: 0000/ { cnt++; }' "$i"; 
done  >> data.py
