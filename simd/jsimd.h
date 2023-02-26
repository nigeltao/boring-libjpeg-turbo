/*
 * simd/jsimd.h
 *
 * Copyright 2009 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright (C) 2011, 2014-2016, 2018, 2020, 2022, D. R. Commander.
 * Copyright (C) 2013-2014, MIPS Technologies, Inc., California.
 * Copyright (C) 2014, Linaro Limited.
 * Copyright (C) 2015-2016, 2018, 2022, Matthieu Darbois.
 * Copyright (C) 2016-2018, Loongson Technology Corporation Limited, BeiJing.
 * Copyright (C) 2020, Arm Limited.
 *
 * Based on the x86 SIMD extension for IJG JPEG library,
 * Copyright (C) 1999-2006, MIYASAKA Masaru.
 * For conditions of distribution and use, see copyright notice in jsimdext.inc
 *
 */

/* Bitmask for supported acceleration methods */

#define JSIMD_NONE     0x00
#define JSIMD_MMX      0x01
#define JSIMD_3DNOW    0x02
#define JSIMD_SSE      0x04
#define JSIMD_SSE2     0x08
#define JSIMD_NEON     0x10
#define JSIMD_DSPR2    0x20
#define JSIMD_ALTIVEC  0x40
#define JSIMD_AVX2     0x80
#define JSIMD_MMI      0x100

/* SIMD Ext: retrieve SIMD/CPU information */
EXTERN(unsigned int) jpeg_simd_cpu_support(void);

/* RGB & extended RGB --> YCC Colorspace Conversion */
extern const int jconst_rgb_ycc_convert_sse2[];
EXTERN(void) jsimd_rgb_ycc_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgb_ycc_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgbx_ycc_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgr_ycc_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgrx_ycc_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxbgr_ycc_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxrgb_ycc_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);

extern const int jconst_rgb_ycc_convert_avx2[];
EXTERN(void) jsimd_rgb_ycc_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgb_ycc_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgbx_ycc_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgr_ycc_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgrx_ycc_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxbgr_ycc_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxrgb_ycc_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);

EXTERN(void) jsimd_rgb_ycc_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgb_ycc_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgbx_ycc_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgr_ycc_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgrx_ycc_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxbgr_ycc_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxrgb_ycc_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);

#ifndef NEON_INTRINSICS

EXTERN(void) jsimd_extrgb_ycc_convert_neon_slowld3
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgr_ycc_convert_neon_slowld3
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);

#endif

/* RGB & extended RGB --> Grayscale Colorspace Conversion */
extern const int jconst_rgb_gray_convert_sse2[];
EXTERN(void) jsimd_rgb_gray_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgb_gray_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgbx_gray_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgr_gray_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgrx_gray_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxbgr_gray_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxrgb_gray_convert_sse2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);

extern const int jconst_rgb_gray_convert_avx2[];
EXTERN(void) jsimd_rgb_gray_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgb_gray_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgbx_gray_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgr_gray_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgrx_gray_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxbgr_gray_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxrgb_gray_convert_avx2
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);

EXTERN(void) jsimd_rgb_gray_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgb_gray_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extrgbx_gray_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgr_gray_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extbgrx_gray_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxbgr_gray_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);
EXTERN(void) jsimd_extxrgb_gray_convert_neon
  (JDIMENSION img_width, JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
   JDIMENSION output_row, int num_rows);

/* YCC --> RGB & extended RGB Colorspace Conversion */
extern const int jconst_ycc_rgb_convert_sse2[];
EXTERN(void) jsimd_ycc_rgb_convert_sse2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extrgb_convert_sse2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extrgbx_convert_sse2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extbgr_convert_sse2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extbgrx_convert_sse2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extxbgr_convert_sse2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extxrgb_convert_sse2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);

extern const int jconst_ycc_rgb_convert_avx2[];
EXTERN(void) jsimd_ycc_rgb_convert_avx2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extrgb_convert_avx2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extrgbx_convert_avx2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extbgr_convert_avx2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extbgrx_convert_avx2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extxbgr_convert_avx2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extxrgb_convert_avx2
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);

EXTERN(void) jsimd_ycc_rgb_convert_neon
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extrgb_convert_neon
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extrgbx_convert_neon
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extbgr_convert_neon
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extbgrx_convert_neon
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extxbgr_convert_neon
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extxrgb_convert_neon
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);

#ifndef NEON_INTRINSICS

EXTERN(void) jsimd_ycc_extrgb_convert_neon_slowst3
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);
EXTERN(void) jsimd_ycc_extbgr_convert_neon_slowst3
  (JDIMENSION out_width, JSAMPIMAGE input_buf, JDIMENSION input_row,
   JSAMPARRAY output_buf, int num_rows);

#endif

/* h2v1 Downsampling */
EXTERN(void) jsimd_h2v1_downsample_sse2
  (JDIMENSION image_width, int max_v_samp_factor, JDIMENSION v_samp_factor,
   JDIMENSION width_in_blocks, JSAMPARRAY input_data, JSAMPARRAY output_data);

EXTERN(void) jsimd_h2v1_downsample_avx2
  (JDIMENSION image_width, int max_v_samp_factor, JDIMENSION v_samp_factor,
   JDIMENSION width_in_blocks, JSAMPARRAY input_data, JSAMPARRAY output_data);

EXTERN(void) jsimd_h2v1_downsample_neon
  (JDIMENSION image_width, int max_v_samp_factor, JDIMENSION v_samp_factor,
   JDIMENSION width_in_blocks, JSAMPARRAY input_data, JSAMPARRAY output_data);

/* h2v2 Downsampling */
EXTERN(void) jsimd_h2v2_downsample_sse2
  (JDIMENSION image_width, int max_v_samp_factor, JDIMENSION v_samp_factor,
   JDIMENSION width_in_blocks, JSAMPARRAY input_data, JSAMPARRAY output_data);

EXTERN(void) jsimd_h2v2_downsample_avx2
  (JDIMENSION image_width, int max_v_samp_factor, JDIMENSION v_samp_factor,
   JDIMENSION width_in_blocks, JSAMPARRAY input_data, JSAMPARRAY output_data);

EXTERN(void) jsimd_h2v2_downsample_neon
  (JDIMENSION image_width, int max_v_samp_factor, JDIMENSION v_samp_factor,
   JDIMENSION width_in_blocks, JSAMPARRAY input_data, JSAMPARRAY output_data);

/* Fancy Upsampling */
extern const int jconst_fancy_upsample_sse2[];
EXTERN(void) jsimd_h2v1_fancy_upsample_sse2
  (int max_v_samp_factor, JDIMENSION downsampled_width, JSAMPARRAY input_data,
   JSAMPARRAY *output_data_ptr);
EXTERN(void) jsimd_h2v2_fancy_upsample_sse2
  (int max_v_samp_factor, JDIMENSION downsampled_width, JSAMPARRAY input_data,
   JSAMPARRAY *output_data_ptr);

extern const int jconst_fancy_upsample_avx2[];
EXTERN(void) jsimd_h2v1_fancy_upsample_avx2
  (int max_v_samp_factor, JDIMENSION downsampled_width, JSAMPARRAY input_data,
   JSAMPARRAY *output_data_ptr);
EXTERN(void) jsimd_h2v2_fancy_upsample_avx2
  (int max_v_samp_factor, JDIMENSION downsampled_width, JSAMPARRAY input_data,
   JSAMPARRAY *output_data_ptr);

EXTERN(void) jsimd_h2v1_fancy_upsample_neon
  (int max_v_samp_factor, JDIMENSION downsampled_width, JSAMPARRAY input_data,
   JSAMPARRAY *output_data_ptr);
EXTERN(void) jsimd_h2v2_fancy_upsample_neon
  (int max_v_samp_factor, JDIMENSION downsampled_width, JSAMPARRAY input_data,
   JSAMPARRAY *output_data_ptr);
EXTERN(void) jsimd_h1v2_fancy_upsample_neon
  (int max_v_samp_factor, JDIMENSION downsampled_width, JSAMPARRAY input_data,
   JSAMPARRAY *output_data_ptr);

/* Sample Conversion */
EXTERN(void) jsimd_convsamp_sse2
  (JSAMPARRAY sample_data, JDIMENSION start_col, DCTELEM *workspace);

EXTERN(void) jsimd_convsamp_avx2
  (JSAMPARRAY sample_data, JDIMENSION start_col, DCTELEM *workspace);

EXTERN(void) jsimd_convsamp_neon
  (JSAMPARRAY sample_data, JDIMENSION start_col, DCTELEM *workspace);

/* Accurate Integer Forward DCT */
extern const int jconst_fdct_islow_sse2[];
EXTERN(void) jsimd_fdct_islow_sse2(DCTELEM *data);

extern const int jconst_fdct_islow_avx2[];
EXTERN(void) jsimd_fdct_islow_avx2(DCTELEM *data);

EXTERN(void) jsimd_fdct_islow_neon(DCTELEM *data);

/* Quantization */
EXTERN(void) jsimd_quantize_sse2
  (JCOEFPTR coef_block, DCTELEM *divisors, DCTELEM *workspace);

EXTERN(void) jsimd_quantize_avx2
  (JCOEFPTR coef_block, DCTELEM *divisors, DCTELEM *workspace);

EXTERN(void) jsimd_quantize_neon
  (JCOEFPTR coef_block, DCTELEM *divisors, DCTELEM *workspace);

/* Accurate Integer Inverse DCT */
extern const int jconst_idct_islow_sse2[];
EXTERN(void) jsimd_idct_islow_sse2
  (void *dct_table, JCOEFPTR coef_block, JSAMPARRAY output_buf,
   JDIMENSION output_col);

extern const int jconst_idct_islow_avx2[];
EXTERN(void) jsimd_idct_islow_avx2
  (void *dct_table, JCOEFPTR coef_block, JSAMPARRAY output_buf,
   JDIMENSION output_col);

EXTERN(void) jsimd_idct_islow_neon
  (void *dct_table, JCOEFPTR coef_block, JSAMPARRAY output_buf,
   JDIMENSION output_col);

/* Huffman coding */
extern const int jconst_huff_encode_one_block[];
EXTERN(JOCTET *) jsimd_huff_encode_one_block_sse2
  (void *state, JOCTET *buffer, JCOEFPTR block, int last_dc_val,
   c_derived_tbl *dctbl, c_derived_tbl *actbl);

EXTERN(JOCTET *) jsimd_huff_encode_one_block_neon
  (void *state, JOCTET *buffer, JCOEFPTR block, int last_dc_val,
   c_derived_tbl *dctbl, c_derived_tbl *actbl);

#ifndef NEON_INTRINSICS

EXTERN(JOCTET *) jsimd_huff_encode_one_block_neon_slowtbl
  (void *state, JOCTET *buffer, JCOEFPTR block, int last_dc_val,
   c_derived_tbl *dctbl, c_derived_tbl *actbl);

#endif

/* Progressive Huffman encoding */
EXTERN(void) jsimd_encode_mcu_AC_first_prepare_sse2
  (const JCOEF *block, const int *jpeg_natural_order_start, int Sl, int Al,
   UJCOEF *values, size_t *zerobits);

EXTERN(void) jsimd_encode_mcu_AC_first_prepare_neon
  (const JCOEF *block, const int *jpeg_natural_order_start, int Sl, int Al,
   UJCOEF *values, size_t *zerobits);

EXTERN(int) jsimd_encode_mcu_AC_refine_prepare_sse2
  (const JCOEF *block, const int *jpeg_natural_order_start, int Sl, int Al,
   UJCOEF *absvalues, size_t *bits);

EXTERN(int) jsimd_encode_mcu_AC_refine_prepare_neon
  (const JCOEF *block, const int *jpeg_natural_order_start, int Sl, int Al,
   UJCOEF *absvalues, size_t *bits);
