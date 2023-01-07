#!/bin/bash -eu

CC=${CC:-gcc}
CFLAGS=${CFLAGS:--O3}

C_FILES=(
    jcapimin.c
    jcapistd.c
    jccoefct.c
    jccolor.c
    jcdctmgr.c
    jchuff.c
    jcicc.c
    jcinit.c
    jcmainct.c
    jcmarker.c
    jcmaster.c
    jcomapi.c
    jcparam.c
    jcphuff.c
    jcprepct.c
    jcsample.c
    jctrans.c
    jdapimin.c
    jdapistd.c
    jdatadst.c
    jdatasrc.c
    jdcoefct.c
    jdcolor.c
    jddctmgr.c
    jdhuff.c
    jdicc.c
    jdinput.c
    jdmainct.c
    jdmarker.c
    jdmaster.c
    jdmerge.c
    jdphuff.c
    jdpostct.c
    jdsample.c
    jdtrans.c
    jerror.c
    jfdctflt.c
    jfdctfst.c
    jfdctint.c
    jidctflt.c
    jidctfst.c
    jidctint.c
    jidctred.c
    jmemmgr.c
    jmemnobs.c
    jquant1.c
    jquant2.c
    jutils.c

    simd/x86_64/jsimd.c
)

ASM_FILES=(
    simd/x86_64/jccolor-avx2.asm
    simd/x86_64/jccolor-sse2.asm
    simd/x86_64/jcgray-avx2.asm
    simd/x86_64/jcgray-sse2.asm
    simd/x86_64/jchuff-sse2.asm
    simd/x86_64/jcphuff-sse2.asm
    simd/x86_64/jcsample-avx2.asm
    simd/x86_64/jcsample-sse2.asm
    simd/x86_64/jdcolor-avx2.asm
    simd/x86_64/jdcolor-sse2.asm
    simd/x86_64/jdmerge-avx2.asm
    simd/x86_64/jdmerge-sse2.asm
    simd/x86_64/jdsample-avx2.asm
    simd/x86_64/jdsample-sse2.asm
    simd/x86_64/jfdctflt-sse.asm
    simd/x86_64/jfdctfst-sse2.asm
    simd/x86_64/jfdctint-avx2.asm
    simd/x86_64/jfdctint-sse2.asm
    simd/x86_64/jidctflt-sse2.asm
    simd/x86_64/jidctfst-sse2.asm
    simd/x86_64/jidctint-avx2.asm
    simd/x86_64/jidctint-sse2.asm
    simd/x86_64/jidctred-sse2.asm
    simd/x86_64/jquantf-sse2.asm
    simd/x86_64/jquanti-avx2.asm
    simd/x86_64/jquanti-sse2.asm
    simd/x86_64/jsimdcpu.asm
)

OBJ_FILES=()

for f in ${C_FILES[@]}; do
    DST=${f%.c}.o
    echo Building $DST
    $CC $CFLAGS -c -I . $f -o $DST
    OBJ_FILES+="$DST "
done

for f in ${ASM_FILES[@]}; do
    DST=${f%.asm}.o
    echo Building $DST
    nasm -I simd/nasm -I simd/x86_64 -D__x86_64__ -DPIC -DELF -f elf64 $f
    OBJ_FILES+="$DST "
done

echo Building cjpeg
$CC $CFLAGS -I . -D PPM_SUPPORTED ${OBJ_FILES[@]} \
    cjpeg.c cdjpeg.c rdppm.c rdswitch.c -o cjpeg

echo Building djpeg
$CC $CFLAGS -I . -D PPM_SUPPORTED ${OBJ_FILES[@]} \
    djpeg.c cdjpeg.c wrppm.c rdcolmap.c -o djpeg
