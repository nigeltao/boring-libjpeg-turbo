/*
 * jpegcomp.h
 *
 * Copyright (C) 2010, 2020, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * JPEG compatibility macros
 * These declarations are considered internal to the JPEG library; most
 * applications using the library shouldn't need to include this file.
 */

#if JPEG_LIB_VERSION >= 70
#define _jpeg_width  jpeg_width
#define _jpeg_height  jpeg_height
#define JERR_ARITH_NOTIMPL  JERR_NOT_COMPILED
#else
#define _jpeg_width  image_width
#define _jpeg_height  image_height
#endif
