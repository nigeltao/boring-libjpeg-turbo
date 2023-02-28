/*
 * cdjpeg.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * Modified 2019 by Guido Vollbeding.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2017, 2019, 2021-2022, D. R. Commander.
 * boring-libjpeg-turbo Modifications:
 * Copyright (C) 2023, Nigel Tao.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains common declarations for the sample applications
 * cjpeg and djpeg.  It is NOT used by the core JPEG library.
 */

#define JPEG_CJPEG_DJPEG        /* define proper options in jconfig.h */
#define JPEG_INTERNAL_OPTIONS   /* cjpeg.c,djpeg.c need to see xxx_SUPPORTED */
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"             /* get library error codes too */
#include "cderror.h"            /* get application-specific error codes */


/*
 * Object interface for cjpeg's source file decoding modules
 */

typedef struct cjpeg_source_struct *cjpeg_source_ptr;

struct cjpeg_source_struct {
  void (*start_input) (j_compress_ptr cinfo, cjpeg_source_ptr sinfo);
  JDIMENSION (*get_pixel_rows) (j_compress_ptr cinfo, cjpeg_source_ptr sinfo);
  void (*finish_input) (j_compress_ptr cinfo, cjpeg_source_ptr sinfo);

  FILE *input_file;

  JSAMPARRAY buffer;
  JDIMENSION buffer_height;
};


/*
 * Object interface for djpeg's output file encoding modules
 */

typedef struct djpeg_dest_struct *djpeg_dest_ptr;

struct djpeg_dest_struct {
  /* start_output is called after jpeg_start_decompress finishes.
   * The color map will be ready at this time, if one is needed.
   */
  void (*start_output) (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo);
  /* Emit the specified number of pixel rows from the buffer. */
  void (*put_pixel_rows) (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
                          JDIMENSION rows_supplied);
  /* Finish up at the end of the image. */
  void (*finish_output) (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo);
  /* Re-calculate buffer dimensions based on output dimensions (for use with
     partial image decompression.)  If this is NULL, then the output format
     does not support partial image decompression (BMP, in particular, cannot
     support partial decompression because it uses an inversion buffer to write
     the image in bottom-up order.) */
  void (*calc_buffer_dimensions) (j_decompress_ptr cinfo,
                                  djpeg_dest_ptr dinfo);


  /* Target file spec; filled in by djpeg.c after object is created. */
  FILE *output_file;

  /* Output pixel-row buffer.  Created by module init or start_output.
   * Width is cinfo->output_width * cinfo->output_components;
   * height is buffer_height.
   */
  JSAMPARRAY buffer;
  JDIMENSION buffer_height;
};


/* Module selection routines for I/O modules. */

EXTERN(cjpeg_source_ptr) jinit_read_ppm(j_compress_ptr cinfo);
EXTERN(djpeg_dest_ptr) jinit_write_ppm(j_decompress_ptr cinfo);

/* miscellaneous useful macros */

#ifndef EXIT_FAILURE            /* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#endif
#ifndef EXIT_WARNING
#define EXIT_WARNING  2
#endif
