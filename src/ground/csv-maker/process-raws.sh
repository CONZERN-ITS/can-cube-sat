#!/bin/bash

for filename in ./raws/*.mavlog; do
    echo "${filename}"
    python make_csv.py -i "${filename}" -o "${filename}-output" --notimestamps
done