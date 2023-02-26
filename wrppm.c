/*
 * wrppm.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * Modified 2009 by Guido Vollbeding.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2017, 2019-2020, 2022, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains routines to write output images in PPM/PGM format.
 * The extended 2-byte-per-sample raw PPM/PGM formats are supported.
 * The PBMPLUS library is NOT required to compile this software
 * (but it is highly useful as a set of PPM image manipulation programs).
 *
 * These routines may need modification for non-Unix environments or
 * specialized applications.  As they stand, they assume output to
 * an ordinary stdio stream.
 */

#include "cmyk.h"
#include "cdjpeg.h"             /* Common decls for cjpeg/djpeg applications */

#if defined(PPM_SUPPORTED) && \
    (BITS_IN_JSAMPLE != 16 || defined(D_LOSSLESS_SUPPORTED))


/*
 * For 12-bit JPEG data, we either downscale the values to 8 bits
 * (to write standard byte-per-sample PPM/PGM files), or output
 * nonstandard word-per-sample PPM/PGM files.  Downscaling is done
 * if PPM_NORAWWORD is defined (this can be done in the Makefile
 * or in jconfig.h).
 * (When the core library supports data precision reduction, a cleaner
 * implementation will be to ask for that instead.)
 */

#define PUTPPMSAMPLE(ptr, v)  *ptr++ = (char)(v)
#define BYTESPERSAMPLE  1
#define PPM_MAXVAL  255


/*
 * When _JSAMPLE is the same size as char, we can just fwrite() the
 * decompressed data to the PPM or PGM file.
 */


/* Private version of data destination object */

typedef struct {
  struct djpeg_dest_struct pub; /* public fields */

  /* Usually these two pointers point to the same place: */
  char *iobuffer;               /* fwrite's I/O buffer */
  _JSAMPROW pixrow;             /* decompressor output buffer */
  size_t buffer_width;          /* width of I/O buffer */
  JDIMENSION samples_per_row;   /* _JSAMPLEs per output row */
} ppm_dest_struct;

typedef ppm_dest_struct *ppm_dest_ptr;


/*
 * Write some pixel data.
 * In this module rows_supplied will always be 1.
 *
 * put_pixel_rows handles the "normal" 8-bit case where the decompressor
 * output buffer is physically the same as the fwrite buffer.
 */

METHODDEF(void)
put_pixel_rows(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
               JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr)dinfo;

  fwrite(dest->iobuffer, 1, dest->buffer_width, dest->pub.output_file);
}


/*
 * This code is used when we have to copy the data and apply a pixel
 * format translation.  Typically this only happens in 12-bit mode.
 */

METHODDEF(void)
copy_pixel_rows(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
                JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr)dinfo;
  register char *bufferptr;
  register _JSAMPROW ptr;

  ptr = dest->pub._buffer[0];
  bufferptr = dest->iobuffer;
  memcpy(bufferptr, ptr, dest->samples_per_row);
  fwrite(dest->iobuffer, 1, dest->buffer_width, dest->pub.output_file);
}


/*
 * Convert extended RGB to RGB.
 */

METHODDEF(void)
put_rgb(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo, JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr)dinfo;
  register char *bufferptr;
  register _JSAMPROW ptr;
  register JDIMENSION col;
  register int rindex = rgb_red[cinfo->out_color_space];
  register int gindex = rgb_green[cinfo->out_color_space];
  register int bindex = rgb_blue[cinfo->out_color_space];
  register int ps = rgb_pixelsize[cinfo->out_color_space];

  ptr = dest->pub._buffer[0];
  bufferptr = dest->iobuffer;
  for (col = cinfo->output_width; col > 0; col--) {
    PUTPPMSAMPLE(bufferptr, ptr[rindex]);
    PUTPPMSAMPLE(bufferptr, ptr[gindex]);
    PUTPPMSAMPLE(bufferptr, ptr[bindex]);
    ptr += ps;
  }
  fwrite(dest->iobuffer, 1, dest->buffer_width, dest->pub.output_file);
}


/*
 * Convert CMYK to RGB.
 */

METHODDEF(void)
put_cmyk(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
         JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr)dinfo;
  register char *bufferptr;
  register _JSAMPROW ptr;
  register JDIMENSION col;

  ptr = dest->pub._buffer[0];
  bufferptr = dest->iobuffer;
  for (col = cinfo->output_width; col > 0; col--) {
    _JSAMPLE r, g, b, c = *ptr++, m = *ptr++, y = *ptr++, k = *ptr++;
    cmyk_to_rgb(c, m, y, k, &r, &g, &b);
    PUTPPMSAMPLE(bufferptr, r);
    PUTPPMSAMPLE(bufferptr, g);
    PUTPPMSAMPLE(bufferptr, b);
  }
  fwrite(dest->iobuffer, 1, dest->buffer_width, dest->pub.output_file);
}


/*
 * Startup: write the file header.
 */

METHODDEF(void)
start_output_ppm(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  ppm_dest_ptr dest = (ppm_dest_ptr)dinfo;

  /* Emit file header */
  switch (cinfo->out_color_space) {
  case JCS_GRAYSCALE:
    /* emit header for raw PGM format */
    fprintf(dest->pub.output_file, "P5\n%ld %ld\n%d\n",
            (long)cinfo->output_width, (long)cinfo->output_height, PPM_MAXVAL);
    break;
  case JCS_RGB:
  case JCS_EXT_RGB:
  case JCS_EXT_RGBX:
  case JCS_EXT_BGR:
  case JCS_EXT_BGRX:
  case JCS_EXT_XBGR:
  case JCS_EXT_XRGB:
  case JCS_EXT_RGBA:
  case JCS_EXT_BGRA:
  case JCS_EXT_ABGR:
  case JCS_EXT_ARGB:
  case JCS_CMYK:
    /* emit header for raw PPM format */
    fprintf(dest->pub.output_file, "P6\n%ld %ld\n%d\n",
            (long)cinfo->output_width, (long)cinfo->output_height, PPM_MAXVAL);
    break;
  default:
    ERREXIT(cinfo, JERR_PPM_COLORSPACE);
  }
}


/*
 * Finish up at the end of the file.
 */

METHODDEF(void)
finish_output_ppm(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  /* Make sure we wrote the output file OK */
  fflush(dinfo->output_file);
  if (ferror(dinfo->output_file))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


/*
 * Re-calculate buffer dimensions based on output dimensions.
 */

METHODDEF(void)
calc_buffer_dimensions_ppm(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  ppm_dest_ptr dest = (ppm_dest_ptr)dinfo;

  if (cinfo->out_color_space == JCS_GRAYSCALE)
    dest->samples_per_row = cinfo->output_width * cinfo->out_color_components;
  else
    dest->samples_per_row = cinfo->output_width * 3;
  dest->buffer_width = dest->samples_per_row * (BYTESPERSAMPLE * sizeof(char));
}


/*
 * The module selection routine for PPM format output.
 */

GLOBAL(djpeg_dest_ptr)
_jinit_write_ppm(j_decompress_ptr cinfo)
{
  ppm_dest_ptr dest;

  if (cinfo->data_precision != BITS_IN_JSAMPLE)
    ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);

  /* Create module interface object, fill in method pointers */
  dest = (ppm_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                  sizeof(ppm_dest_struct));
  dest->pub.start_output = start_output_ppm;
  dest->pub.finish_output = finish_output_ppm;
  dest->pub.calc_buffer_dimensions = calc_buffer_dimensions_ppm;

  /* Calculate output image dimensions so we can allocate space */
  jpeg_calc_output_dimensions(cinfo);

  /* Create physical I/O buffer */
  dest->pub.calc_buffer_dimensions(cinfo, (djpeg_dest_ptr)dest);
  dest->iobuffer = (char *)(*cinfo->mem->alloc_small)
    ((j_common_ptr)cinfo, JPOOL_IMAGE, dest->buffer_width);

  if (sizeof(_JSAMPLE) != sizeof(char) ||
#if RGB_RED == 0 && RGB_GREEN == 1 && RGB_BLUE == 2 && RGB_PIXELSIZE == 3
      (cinfo->out_color_space != JCS_EXT_RGB &&
       cinfo->out_color_space != JCS_RGB)) {
#else
      cinfo->out_color_space != JCS_EXT_RGB) {
#endif
    /* When quantizing, we need an output buffer for colormap indexes
     * that's separate from the physical I/O buffer.  We also need a
     * separate buffer if pixel format translation must take place.
     */
    dest->pub._buffer = (_JSAMPARRAY)(*cinfo->mem->alloc_sarray)
      ((j_common_ptr)cinfo, JPOOL_IMAGE,
       cinfo->output_width * cinfo->output_components, (JDIMENSION)1);
    dest->pub.buffer_height = 1;
    if (BORING_ALWAYS_TRUE) {
      if (IsExtRGB(cinfo->out_color_space))
        dest->pub.put_pixel_rows = put_rgb;
      else if (cinfo->out_color_space == JCS_CMYK)
        dest->pub.put_pixel_rows = put_cmyk;
      else
        dest->pub.put_pixel_rows = copy_pixel_rows;
    }
  } else {
    /* We will fwrite() directly from decompressor output buffer. */
    /* Synthesize a _JSAMPARRAY pointer structure */
    dest->pixrow = (_JSAMPROW)dest->iobuffer;
    dest->pub._buffer = &dest->pixrow;
    dest->pub.buffer_height = 1;
    dest->pub.put_pixel_rows = put_pixel_rows;
  }

  return (djpeg_dest_ptr)dest;
}

#endif /* defined(PPM_SUPPORTED) &&
          (BITS_IN_JSAMPLE != 16 || defined(D_LOSSLESS_SUPPORTED)) */
