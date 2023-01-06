#!/bin/bash -eu

# This is just a very basic consistency check, comparing the cjpeg and djpeg
# programs' output on a single golden test case each.

if [ ! -f ./cjpeg ] || [ ! -f ./djpeg ]; then
    echo "./cjpeg or ./djpeg not found; run a build-*.sh script first"
    exit 1
fi

CHASH=$(./cjpeg testimages/testorig.ppm | sha256sum | cut -d ' ' -f1)
if [ $CHASH != "491679b8057739b3c8e5bacd1e918efb1691d271cbbd69820ff8d480dcb90963" ]; then
    echo "./cjpeg produced unexpected output"
    exit 1
fi

DHASH=$(./djpeg testimages/testorig.jpg | sha256sum | cut -d ' ' -f1)
if [ $DHASH != "b64a47d529e289e5570648db9f0385d9b240448724cdde7716fa0139b121d491" ]; then
    echo "./djpeg produced unexpected output"
    exit 1
fi

echo PASS
