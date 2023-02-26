/*
 * jidctint.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1998, Thomas G. Lane.
 * Modification developed 2002-2018 by Guido Vollbeding.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2015, 2020, 2022, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains a slower but more accurate integer implementation of the
 * inverse DCT (Discrete Cosine Transform).  In the IJG code, this routine
 * must also perform dequantization of the input coefficients.
 *
 * A 2-D IDCT can be done by 1-D IDCT on each column followed by 1-D IDCT
 * on each row (or vice versa, but it's more convenient to emit a row at
 * a time).  Direct algorithms are also available, but they are much more
 * complex and seem not to be any faster when reduced to code.
 *
 * This implementation is based on an algorithm described in
 *   C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
 *   Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
 *   Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
 * The primary algorithm described there uses 11 multiplies and 29 adds.
 * We use their alternate method with 12 multiplies and 32 adds.
 * The advantage of this method is that no data path contains more than one
 * multiplication; this allows a very simple and accurate implementation in
 * scaled fixed-point arithmetic, with a minimal number of shifts.
 *
 * We also provide IDCT routines with various output sample block sizes for
 * direct resolution reduction or enlargement without additional resampling:
 * NxN (N=1...16) pixels for one 8x8 input DCT block.
 *
 * For N<8 we simply take the corresponding low-frequency coefficients of
 * the 8x8 input DCT block and apply an NxN point IDCT on the sub-block
 * to yield the downscaled outputs.
 * This can be seen as direct low-pass downsampling from the DCT domain
 * point of view rather than the usual spatial domain point of view,
 * yielding significant computational savings and results at least
 * as good as common bilinear (averaging) spatial downsampling.
 *
 * For N>8 we apply a partial NxN IDCT on the 8 input coefficients as
 * lower frequencies and higher frequencies assumed to be zero.
 * It turns out that the computational effort is similar to the 8x8 IDCT
 * regarding the output size.
 * Furthermore, the scaling and descaling is the same for all IDCT sizes.
 *
 * CAUTION: We rely on the FIX() macro except for the N=1,2,4,8 cases
 * since there would be too many additional constants to pre-calculate.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"               /* Private declarations for DCT subsystem */

#ifdef DCT_ISLOW_SUPPORTED


/*
 * This module is specialized to the case DCTSIZE = 8.
 */

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCT blocks. /* deliberate syntax err */
#endif


/*
 * The poop on this scaling stuff is as follows:
 *
 * Each 1-D IDCT step produces outputs which are a factor of sqrt(N)
 * larger than the true IDCT outputs.  The final outputs are therefore
 * a factor of N larger than desired; since N=8 this can be cured by
 * a simple right shift at the end of the algorithm.  The advantage of
 * this arrangement is that we save two multiplications per 1-D IDCT,
 * because the y0 and y4 inputs need not be divided by sqrt(N).
 *
 * We have to do addition and subtraction of the integer inputs, which
 * is no problem, and multiplication by fractional constants, which is
 * a problem to do in integer arithmetic.  We multiply all the constants
 * by CONST_SCALE and convert them to integer constants (thus retaining
 * CONST_BITS bits of precision in the constants).  After doing a
 * multiplication we have to divide the product by CONST_SCALE, with proper
 * rounding, to produce the correct output.  This division can be done
 * cheaply as a right shift of CONST_BITS bits.  We postpone shifting
 * as long as possible so that partial sums can be added together with
 * full fractional precision.
 *
 * The outputs of the first pass are scaled up by PASS1_BITS bits so that
 * they are represented to better-than-integral precision.  These outputs
 * require BITS_IN_JSAMPLE + PASS1_BITS + 3 bits; this fits in a 16-bit word
 * with the recommended scaling.  (To scale up 12-bit sample data further, an
 * intermediate JLONG array would be needed.)
 *
 * To avoid overflow of the 32-bit intermediate results in pass 2, we must
 * have BITS_IN_JSAMPLE + CONST_BITS + PASS1_BITS <= 26.  Error analysis
 * shows that the values given below are the most effective.
 */

#define CONST_BITS  13
#define PASS1_BITS  2

/* Some C compilers fail to reduce "FIX(constant)" at compile time, thus
 * causing a lot of useless floating-point operations at run time.
 * To get around this we use the following pre-calculated constants.
 * If you change CONST_BITS you may want to add appropriate values.
 * (With a reasonable C compiler, you can just rely on the FIX() macro...)
 */

#if CONST_BITS == 13
#define FIX_0_298631336  ((JLONG)2446)          /* FIX(0.298631336) */
#define FIX_0_390180644  ((JLONG)3196)          /* FIX(0.390180644) */
#define FIX_0_541196100  ((JLONG)4433)          /* FIX(0.541196100) */
#define FIX_0_765366865  ((JLONG)6270)          /* FIX(0.765366865) */
#define FIX_0_899976223  ((JLONG)7373)          /* FIX(0.899976223) */
#define FIX_1_175875602  ((JLONG)9633)          /* FIX(1.175875602) */
#define FIX_1_501321110  ((JLONG)12299)         /* FIX(1.501321110) */
#define FIX_1_847759065  ((JLONG)15137)         /* FIX(1.847759065) */
#define FIX_1_961570560  ((JLONG)16069)         /* FIX(1.961570560) */
#define FIX_2_053119869  ((JLONG)16819)         /* FIX(2.053119869) */
#define FIX_2_562915447  ((JLONG)20995)         /* FIX(2.562915447) */
#define FIX_3_072711026  ((JLONG)25172)         /* FIX(3.072711026) */
#else
#define FIX_0_298631336  FIX(0.298631336)
#define FIX_0_390180644  FIX(0.390180644)
#define FIX_0_541196100  FIX(0.541196100)
#define FIX_0_765366865  FIX(0.765366865)
#define FIX_0_899976223  FIX(0.899976223)
#define FIX_1_175875602  FIX(1.175875602)
#define FIX_1_501321110  FIX(1.501321110)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_1_961570560  FIX(1.961570560)
#define FIX_2_053119869  FIX(2.053119869)
#define FIX_2_562915447  FIX(2.562915447)
#define FIX_3_072711026  FIX(3.072711026)
#endif


/* Multiply an JLONG variable by an JLONG constant to yield an JLONG result.
 * For 8-bit samples with the recommended scaling, all the variable
 * and constant values involved are no more than 16 bits wide, so a
 * 16x16->32 bit multiply can be used instead of a full 32x32 multiply.
 * For 12-bit samples, a full 32-bit multiplication will be needed.
 */

#define MULTIPLY(var, const)  MULTIPLY16C16(var, const)


/* Dequantize a coefficient by multiplying it by the multiplier-table
 * entry; produce an int result.  In this module, both inputs and result
 * are 16 bits or less, so either int or short multiply will work.
 */

#define DEQUANTIZE(coef, quantval)  (((ISLOW_MULT_TYPE)(coef)) * (quantval))


/*
 * Perform dequantization and inverse DCT on one block of coefficients.
 */

GLOBAL(void)
_jpeg_idct_islow(j_decompress_ptr cinfo, jpeg_component_info *compptr,
                 JCOEFPTR coef_block, _JSAMPARRAY output_buf,
                 JDIMENSION output_col)
{
  JLONG tmp0, tmp1, tmp2, tmp3;
  JLONG tmp10, tmp11, tmp12, tmp13;
  JLONG z1, z2, z3, z4, z5;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE *quantptr;
  int *wsptr;
  _JSAMPROW outptr;
  _JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[DCTSIZE2];      /* buffers data between passes */

  /* Pass 1: process columns from input, store into work array. */
  /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *)compptr->dct_table;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; ctr--) {
    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any column in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * column DCT calculations can be simplified this way.
     */

    if (inptr[DCTSIZE * 1] == 0 && inptr[DCTSIZE * 2] == 0 &&
        inptr[DCTSIZE * 3] == 0 && inptr[DCTSIZE * 4] == 0 &&
        inptr[DCTSIZE * 5] == 0 && inptr[DCTSIZE * 6] == 0 &&
        inptr[DCTSIZE * 7] == 0) {
      /* AC terms all zero */
      int dcval = LEFT_SHIFT(DEQUANTIZE(inptr[DCTSIZE * 0],
                             quantptr[DCTSIZE * 0]), PASS1_BITS);

      wsptr[DCTSIZE * 0] = dcval;
      wsptr[DCTSIZE * 1] = dcval;
      wsptr[DCTSIZE * 2] = dcval;
      wsptr[DCTSIZE * 3] = dcval;
      wsptr[DCTSIZE * 4] = dcval;
      wsptr[DCTSIZE * 5] = dcval;
      wsptr[DCTSIZE * 6] = dcval;
      wsptr[DCTSIZE * 7] = dcval;

      inptr++;                  /* advance pointers to next column */
      quantptr++;
      wsptr++;
      continue;
    }

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = DEQUANTIZE(inptr[DCTSIZE * 2], quantptr[DCTSIZE * 2]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 6], quantptr[DCTSIZE * 6]);

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, -FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    z2 = DEQUANTIZE(inptr[DCTSIZE * 0], quantptr[DCTSIZE * 0]);
    z3 = DEQUANTIZE(inptr[DCTSIZE * 4], quantptr[DCTSIZE * 4]);

    tmp0 = LEFT_SHIFT(z2 + z3, CONST_BITS);
    tmp1 = LEFT_SHIFT(z2 - z3, CONST_BITS);

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = DEQUANTIZE(inptr[DCTSIZE * 7], quantptr[DCTSIZE * 7]);
    tmp1 = DEQUANTIZE(inptr[DCTSIZE * 5], quantptr[DCTSIZE * 5]);
    tmp2 = DEQUANTIZE(inptr[DCTSIZE * 3], quantptr[DCTSIZE * 3]);
    tmp3 = DEQUANTIZE(inptr[DCTSIZE * 1], quantptr[DCTSIZE * 1]);

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, -FIX_0_899976223); /* sqrt(2) * ( c7-c3) */
    z2 = MULTIPLY(z2, -FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, -FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, -FIX_0_390180644); /* sqrt(2) * ( c5-c3) */

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    wsptr[DCTSIZE * 0] = (int)DESCALE(tmp10 + tmp3, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 7] = (int)DESCALE(tmp10 - tmp3, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 1] = (int)DESCALE(tmp11 + tmp2, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 6] = (int)DESCALE(tmp11 - tmp2, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 2] = (int)DESCALE(tmp12 + tmp1, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 5] = (int)DESCALE(tmp12 - tmp1, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 3] = (int)DESCALE(tmp13 + tmp0, CONST_BITS - PASS1_BITS);
    wsptr[DCTSIZE * 4] = (int)DESCALE(tmp13 - tmp0, CONST_BITS - PASS1_BITS);

    inptr++;                    /* advance pointers to next column */
    quantptr++;
    wsptr++;
  }

  /* Pass 2: process rows from work array, store into output array. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  wsptr = workspace;
  for (ctr = 0; ctr < DCTSIZE; ctr++) {
    outptr = output_buf[ctr] + output_col;
    /* Rows of zeroes can be exploited in the same way as we did with columns.
     * However, the column calculation has created many nonzero AC terms, so
     * the simplification applies less often (typically 5% to 10% of the time).
     * On machines with very fast multiplication, it's possible that the
     * test takes more time than it's worth.  In that case this section
     * may be commented out.
     */

#ifndef NO_ZERO_ROW_TEST
    if (wsptr[1] == 0 && wsptr[2] == 0 && wsptr[3] == 0 && wsptr[4] == 0 &&
        wsptr[5] == 0 && wsptr[6] == 0 && wsptr[7] == 0) {
      /* AC terms all zero */
      _JSAMPLE dcval = range_limit[(int)DESCALE((JLONG)wsptr[0],
                                                PASS1_BITS + 3) & RANGE_MASK];

      outptr[0] = dcval;
      outptr[1] = dcval;
      outptr[2] = dcval;
      outptr[3] = dcval;
      outptr[4] = dcval;
      outptr[5] = dcval;
      outptr[6] = dcval;
      outptr[7] = dcval;

      wsptr += DCTSIZE;         /* advance pointer to next row */
      continue;
    }
#endif

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = (JLONG)wsptr[2];
    z3 = (JLONG)wsptr[6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, -FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = LEFT_SHIFT((JLONG)wsptr[0] + (JLONG)wsptr[4], CONST_BITS);
    tmp1 = LEFT_SHIFT((JLONG)wsptr[0] - (JLONG)wsptr[4], CONST_BITS);

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = (JLONG)wsptr[7];
    tmp1 = (JLONG)wsptr[5];
    tmp2 = (JLONG)wsptr[3];
    tmp3 = (JLONG)wsptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, -FIX_0_899976223); /* sqrt(2) * ( c7-c3) */
    z2 = MULTIPLY(z2, -FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, -FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, -FIX_0_390180644); /* sqrt(2) * ( c5-c3) */

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    outptr[0] = range_limit[(int)DESCALE(tmp10 + tmp3,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[7] = range_limit[(int)DESCALE(tmp10 - tmp3,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[1] = range_limit[(int)DESCALE(tmp11 + tmp2,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[6] = range_limit[(int)DESCALE(tmp11 - tmp2,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[2] = range_limit[(int)DESCALE(tmp12 + tmp1,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[5] = range_limit[(int)DESCALE(tmp12 - tmp1,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[3] = range_limit[(int)DESCALE(tmp13 + tmp0,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];
    outptr[4] = range_limit[(int)DESCALE(tmp13 - tmp0,
                                         CONST_BITS + PASS1_BITS + 3) &
                            RANGE_MASK];

    wsptr += DCTSIZE;           /* advance pointer to next row */
  }
}

#endif /* DCT_ISLOW_SUPPORTED */
