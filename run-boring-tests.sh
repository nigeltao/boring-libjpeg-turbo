#!/bin/bash -eu

# This is just a very basic consistency check, comparing the cjpeg and djpeg
# programs' output on a small suite of golden test cases.

if [ ! -f ./boring-cjpeg ] || [ ! -f ./boring-djpeg ]; then
    echo "./boring-cjpeg or ./boring-djpeg not found; run a build-*.sh script first"
    exit 1
fi

while read LINE; do
    A=($LINE)
    if [[ -z $LINE || ${A[0]} == "#" ]]; then
        continue
    elif [[ ${A[1]} == *.ppm ]]; then
        HASH=$(./boring-cjpeg ${A[1]} | sha256sum | cut -d ' ' -f1)
    elif [[ ${A[1]} == *.jpeg || ${A[1]} == *.jpg ]]; then
        HASH=$(./boring-djpeg ${A[1]} | sha256sum | cut -d ' ' -f1)
    else
        echo "UNEXPECTED $LINE"
        exit 1
    fi
    if [[ $HASH != ${A[0]} ]]; then
        echo "HAVE $HASH"
        echo "WANT ${A[0]}"
        echo "FAIL ${A[1]}"
        exit 1
    fi
done <testimages/boring-tests-hashes.txt

echo PASS
