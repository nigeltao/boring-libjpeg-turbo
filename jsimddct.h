/*
 * jsimddct.h
 *
 * Copyright 2009 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * boring-libjpeg-turbo Modifications:
 * Copyright (C) 2023, Nigel Tao.
 *
 * Based on the x86 SIMD extension for IJG JPEG library,
 * Copyright (C) 1999-2006, MIYASAKA Masaru.
 * For conditions of distribution and use, see copyright notice in jsimdext.inc
 *
 */

EXTERN(int) jsimd_can_convsamp(void);

EXTERN(void) jsimd_convsamp(JSAMPARRAY sample_data, JDIMENSION start_col,
                            DCTELEM *workspace);

EXTERN(int) jsimd_can_fdct_islow(void);

EXTERN(void) jsimd_fdct_islow(DCTELEM *data);

EXTERN(int) jsimd_can_quantize(void);

EXTERN(void) jsimd_quantize(JCOEFPTR coef_block, DCTELEM *divisors,
                            DCTELEM *workspace);

EXTERN(int) jsimd_can_idct_islow(void);

EXTERN(void) jsimd_idct_islow(j_decompress_ptr cinfo,
                              jpeg_component_info *compptr,
                              JCOEFPTR coef_block, JSAMPARRAY output_buf,
                              JDIMENSION output_col);
