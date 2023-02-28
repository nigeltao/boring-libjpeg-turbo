/*
 * jsamplecomp.h
 *
 * Copyright (C) 2022, D. R. Commander.
 * boring-libjpeg-turbo Modifications:
 * Copyright (C) 2023, Nigel Tao.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 */

/* In source files that must be compiled for multiple data precisions, we
 * prefix all precision-dependent data types, macros, methods, fields, and
 * function names with an underscore.  Including this file replaces those
 * precision-independent tokens with their precision-dependent equivalents,
 * based on the value of BITS_IN_JSAMPLE.
 */

#ifndef JSAMPLECOMP_H
#define JSAMPLECOMP_H

/* Sample data types and macros (jmorecfg.h) */
#define _JSAMPLE  JSAMPLE

#define _MAXJSAMPLE  MAXJSAMPLE
#define _CENTERJSAMPLE   CENTERJSAMPLE

#define _JSAMPROW  JSAMPROW
#define _JSAMPARRAY  JSAMPARRAY
#define _JSAMPIMAGE  JSAMPIMAGE

/* External functions (jpeglib.h) */
#define _jpeg_write_scanlines  jpeg_write_scanlines
#define _jpeg_write_raw_data  jpeg_write_raw_data
#define _jpeg_read_scanlines  jpeg_read_scanlines
#define _jpeg_skip_scanlines  jpeg_skip_scanlines
#define _jpeg_crop_scanline  jpeg_crop_scanline
#define _jpeg_read_raw_data  jpeg_read_raw_data

/* Internal methods (jpegint.h) */

/* Use the 8-bit method in the jpeg_c_main_controller structure. */
#define _process_data  process_data
/* Use the 8-bit method in the jpeg_c_prep_controller structure. */
#define _pre_process_data  pre_process_data
/* Use the 8-bit method in the jpeg_c_coef_controller structure. */
#define _compress_data  compress_data
/* Use the 8-bit method in the jpeg_color_converter structure. */
#define _color_convert  color_convert
/* Use the 8-bit method in the jpeg_downsampler structure. */
#define _downsample  downsample
/* Use the 8-bit method in the jpeg_forward_dct structure. */
#define _forward_DCT  forward_DCT
/* Use the 8-bit method in the jpeg_d_main_controller structure. */
#define _process_data  process_data
/* Use the 8-bit method in the jpeg_d_coef_controller structure. */
#define _decompress_data  decompress_data
/* Use the 8-bit method in the jpeg_d_post_controller structure. */
#define _post_process_data  post_process_data
/* Use the 8-bit method in the jpeg_inverse_dct structure. */
#define _inverse_DCT_method_ptr  inverse_DCT_method_ptr
#define _inverse_DCT  inverse_DCT
/* Use the 8-bit method in the jpeg_upsampler structure. */
#define _upsample  upsample
/* Use the 8-bit method in the jpeg_color_converter structure. */
#define _color_convert  color_convert
/* Use the 8-bit method in the jpeg_color_quantizer structure. */
#define _color_quantize  color_quantize

/* Global internal functions (jpegint.h) */
#define _jinit_c_main_controller  jinit_c_main_controller
#define _jinit_c_prep_controller  jinit_c_prep_controller
#define _jinit_c_coef_controller  jinit_c_coef_controller
#define _jinit_color_converter  jinit_color_converter
#define _jinit_downsampler  jinit_downsampler
#define _jinit_forward_dct  jinit_forward_dct

#define _jinit_d_main_controller  jinit_d_main_controller
#define _jinit_d_coef_controller  jinit_d_coef_controller
#define _jinit_d_post_controller  jinit_d_post_controller
#define _jinit_inverse_dct  jinit_inverse_dct
#define _jinit_upsampler  jinit_upsampler
#define _jinit_color_deconverter  jinit_color_deconverter
#define _jinit_1pass_quantizer  jinit_1pass_quantizer
#define _jinit_2pass_quantizer  jinit_2pass_quantizer
#define _jinit_merged_upsampler  jinit_merged_upsampler

#define _jcopy_sample_rows  jcopy_sample_rows

/* Global internal functions (jdct.h) */
#define _jpeg_fdct_islow  jpeg_fdct_islow
#define _jpeg_fdct_ifast  jpeg_fdct_ifast

#define _jpeg_idct_islow  jpeg_idct_islow
#define _jpeg_idct_ifast  jpeg_idct_ifast
#define _jpeg_idct_float  jpeg_idct_float
#define _jpeg_idct_7x7  jpeg_idct_7x7
#define _jpeg_idct_6x6  jpeg_idct_6x6
#define _jpeg_idct_5x5  jpeg_idct_5x5
#define _jpeg_idct_4x4  jpeg_idct_4x4
#define _jpeg_idct_3x3  jpeg_idct_3x3
#define _jpeg_idct_2x2  jpeg_idct_2x2
#define _jpeg_idct_1x1  jpeg_idct_1x1
#define _jpeg_idct_9x9  jpeg_idct_9x9
#define _jpeg_idct_10x10  jpeg_idct_10x10
#define _jpeg_idct_11x11  jpeg_idct_11x11
#define _jpeg_idct_12x12  jpeg_idct_12x12
#define _jpeg_idct_13x13  jpeg_idct_13x13
#define _jpeg_idct_14x14  jpeg_idct_14x14
#define _jpeg_idct_15x15  jpeg_idct_15x15
#define _jpeg_idct_16x16  jpeg_idct_16x16

/* Internal fields (cdjpeg.h) */

/* Use the 8-bit buffer in the cjpeg_source_struct and djpeg_dest_struct
   structures. */
#define _buffer  buffer

/* Image I/O functions (cdjpeg.h) */
#define _jinit_read_gif  jinit_read_gif
#define _jinit_write_gif  jinit_write_gif
#define _jinit_read_ppm  jinit_read_ppm
#define _jinit_write_ppm  jinit_write_ppm

#define _read_color_map  read_color_map

#endif /* JSAMPLECOMP_H */
