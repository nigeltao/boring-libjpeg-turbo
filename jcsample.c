/*
 * jcsample.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * Lossless JPEG Modifications:
 * Copyright (C) 1999, Ken Murchison.
 * libjpeg-turbo Modifications:
 * Copyright 2009 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright (C) 2014, MIPS Technologies, Inc., California.
 * Copyright (C) 2015, 2019, 2022, D. R. Commander.
 * boring-libjpeg-turbo Modifications:
 * Copyright (C) 2023, Nigel Tao.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains downsampling routines.
 *
 * Downsampling input data is counted in "row groups".  A row group
 * is defined to be max_v_samp_factor pixel rows of each component,
 * from which the downsampler produces v_samp_factor sample rows.
 * A single row group is processed in each call to the downsampler module.
 *
 * The downsampler is responsible for edge-expansion of its output data
 * to fill an integral number of DCT blocks horizontally.  The source buffer
 * may be modified if it is helpful for this purpose (the source buffer is
 * allocated wide enough to correspond to the desired output width).
 * The caller (the prep controller) is responsible for vertical padding.
 *
 * The downsampler may request "context rows" by setting need_context_rows
 * during startup.  In this case, the input arrays will contain at least
 * one row group's worth of pixels above and below the passed-in data;
 * the caller will create dummy rows at image top and bottom by replicating
 * the first or last real pixel row.
 *
 * An excellent reference for image resampling is
 *   Digital Image Warping, George Wolberg, 1990.
 *   Pub. by IEEE Computer Society Press, Los Alamitos, CA. ISBN 0-8186-8944-7.
 *
 * The downsampling algorithm used here is a simple average of the source
 * pixels covered by the output pixel.  The hi-falutin sampling literature
 * refers to this as a "box filter".  In general the characteristics of a box
 * filter are not very good, but for the specific cases we normally use (1:1
 * and 2:1 ratios) the box is equivalent to a "triangle filter" which is not
 * nearly so bad.  If you intend to use other sampling ratios, you'd be well
 * advised to improve this code.
 *
 * A simple input-smoothing capability is provided.  This is mainly intended
 * for cleaning up color-dithered GIF input files (if you find it inadequate,
 * we suggest using an external filtering program such as pnmconvol).  When
 * enabled, each input pixel P is replaced by a weighted sum of itself and its
 * eight neighbors.  P's weight is 1-8*SF and each neighbor's weight is SF,
 * where SF = (smoothing_factor / 1024).
 * Currently, smoothing is only supported for 2h2v sampling factors.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jsimd.h"
#include "jsamplecomp.h"


/* Pointer to routine to downsample a single component */
typedef void (*downsample1_ptr) (j_compress_ptr cinfo,
                                 jpeg_component_info *compptr,
                                 _JSAMPARRAY input_data,
                                 _JSAMPARRAY output_data);

/* Private subobject */

typedef struct {
  struct jpeg_downsampler pub;  /* public fields */

  /* Downsampling method pointers, one per component */
  downsample1_ptr methods[MAX_COMPONENTS];
} my_downsampler;

typedef my_downsampler *my_downsample_ptr;


/*
 * Initialize for a downsampling pass.
 */

METHODDEF(void)
start_pass_downsample(j_compress_ptr cinfo)
{
  /* no work for now */
}


/*
 * Expand a component horizontally from width input_cols to width output_cols,
 * by duplicating the rightmost samples.
 */

LOCAL(void)
expand_right_edge(_JSAMPARRAY image_data, int num_rows, JDIMENSION input_cols,
                  JDIMENSION output_cols)
{
  register _JSAMPROW ptr;
  register _JSAMPLE pixval;
  register int count;
  int row;
  int numcols = (int)(output_cols - input_cols);

  if (numcols > 0) {
    for (row = 0; row < num_rows; row++) {
      ptr = image_data[row] + input_cols;
      pixval = ptr[-1];
      for (count = numcols; count > 0; count--)
        *ptr++ = pixval;
    }
  }
}


/*
 * Do downsampling for a whole row group (all components).
 *
 * In this version we simply downsample each component independently.
 */

METHODDEF(void)
sep_downsample(j_compress_ptr cinfo, _JSAMPIMAGE input_buf,
               JDIMENSION in_row_index, _JSAMPIMAGE output_buf,
               JDIMENSION out_row_group_index)
{
  my_downsample_ptr downsample = (my_downsample_ptr)cinfo->downsample;
  int ci;
  jpeg_component_info *compptr;
  _JSAMPARRAY in_ptr, out_ptr;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    in_ptr = input_buf[ci] + in_row_index;
    out_ptr = output_buf[ci] + (out_row_group_index * compptr->v_samp_factor);
    (*downsample->methods[ci]) (cinfo, compptr, in_ptr, out_ptr);
  }
}


/*
 * Downsample pixel values of a single component.
 * One row group is processed per call.
 * This version handles arbitrary integral sampling ratios, without smoothing.
 * Note that this version is not actually used for customary sampling ratios.
 */

METHODDEF(void)
int_downsample(j_compress_ptr cinfo, jpeg_component_info *compptr,
               _JSAMPARRAY input_data, _JSAMPARRAY output_data)
{
  int inrow, outrow, h_expand, v_expand, numpix, numpix2, h, v;
  JDIMENSION outcol, outcol_h;  /* outcol_h == outcol*h_expand */
  JDIMENSION output_cols = compptr->width_in_blocks * DCTSIZE;
  _JSAMPROW inptr, outptr;
  JLONG outvalue;

  h_expand = cinfo->max_h_samp_factor / compptr->h_samp_factor;
  v_expand = cinfo->max_v_samp_factor / compptr->v_samp_factor;
  numpix = h_expand * v_expand;
  numpix2 = numpix / 2;

  /* Expand input data enough to let all the output samples be generated
   * by the standard loop.  Special-casing padded output would be more
   * efficient.
   */
  expand_right_edge(input_data, cinfo->max_v_samp_factor, cinfo->image_width,
                    output_cols * h_expand);

  inrow = 0;
  for (outrow = 0; outrow < compptr->v_samp_factor; outrow++) {
    outptr = output_data[outrow];
    for (outcol = 0, outcol_h = 0; outcol < output_cols;
         outcol++, outcol_h += h_expand) {
      outvalue = 0;
      for (v = 0; v < v_expand; v++) {
        inptr = input_data[inrow + v] + outcol_h;
        for (h = 0; h < h_expand; h++) {
          outvalue += (JLONG)(*inptr++);
        }
      }
      *outptr++ = (_JSAMPLE)((outvalue + numpix2) / numpix);
    }
    inrow += v_expand;
  }
}


/*
 * Downsample pixel values of a single component.
 * This version handles the special case of a full-size component,
 * without smoothing.
 */

METHODDEF(void)
fullsize_downsample(j_compress_ptr cinfo, jpeg_component_info *compptr,
                    _JSAMPARRAY input_data, _JSAMPARRAY output_data)
{
  /* Copy the data */
  _jcopy_sample_rows(input_data, 0, output_data, 0, cinfo->max_v_samp_factor,
                     cinfo->image_width);
  /* Edge-expand */
  expand_right_edge(output_data, cinfo->max_v_samp_factor, cinfo->image_width,
                    compptr->width_in_blocks * DCTSIZE);
}


/*
 * Downsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 1:1 vertical,
 * without smoothing.
 *
 * A note about the "bias" calculations: when rounding fractional values to
 * integer, we do not want to always round 0.5 up to the next integer.
 * If we did that, we'd introduce a noticeable bias towards larger values.
 * Instead, this code is arranged so that 0.5 will be rounded up or down at
 * alternate pixel locations (a simple ordered dither pattern).
 */

METHODDEF(void)
h2v1_downsample(j_compress_ptr cinfo, jpeg_component_info *compptr,
                _JSAMPARRAY input_data, _JSAMPARRAY output_data)
{
  int outrow;
  JDIMENSION outcol;
  JDIMENSION output_cols = compptr->width_in_blocks * DCTSIZE;
  register _JSAMPROW inptr, outptr;
  register int bias;

  /* Expand input data enough to let all the output samples be generated
   * by the standard loop.  Special-casing padded output would be more
   * efficient.
   */
  expand_right_edge(input_data, cinfo->max_v_samp_factor, cinfo->image_width,
                    output_cols * 2);

  for (outrow = 0; outrow < compptr->v_samp_factor; outrow++) {
    outptr = output_data[outrow];
    inptr = input_data[outrow];
    bias = 0;                   /* bias = 0,1,0,1,... for successive samples */
    for (outcol = 0; outcol < output_cols; outcol++) {
      *outptr++ = (_JSAMPLE)((inptr[0] + inptr[1] + bias) >> 1);
      bias ^= 1;                /* 0=>1, 1=>0 */
      inptr += 2;
    }
  }
}


/*
 * Downsample pixel values of a single component.
 * This version handles the standard case of 2:1 horizontal and 2:1 vertical,
 * without smoothing.
 */

METHODDEF(void)
h2v2_downsample(j_compress_ptr cinfo, jpeg_component_info *compptr,
                _JSAMPARRAY input_data, _JSAMPARRAY output_data)
{
  int inrow, outrow;
  JDIMENSION outcol;
  JDIMENSION output_cols = compptr->width_in_blocks * DCTSIZE;
  register _JSAMPROW inptr0, inptr1, outptr;
  register int bias;

  /* Expand input data enough to let all the output samples be generated
   * by the standard loop.  Special-casing padded output would be more
   * efficient.
   */
  expand_right_edge(input_data, cinfo->max_v_samp_factor, cinfo->image_width,
                    output_cols * 2);

  inrow = 0;
  for (outrow = 0; outrow < compptr->v_samp_factor; outrow++) {
    outptr = output_data[outrow];
    inptr0 = input_data[inrow];
    inptr1 = input_data[inrow + 1];
    bias = 1;                   /* bias = 1,2,1,2,... for successive samples */
    for (outcol = 0; outcol < output_cols; outcol++) {
      *outptr++ = (_JSAMPLE)
        ((inptr0[0] + inptr0[1] + inptr1[0] + inptr1[1] + bias) >> 2);
      bias ^= 3;                /* 1=>2, 2=>1 */
      inptr0 += 2;  inptr1 += 2;
    }
    inrow += 2;
  }
}


/*
 * Module initialization routine for downsampling.
 * Note that we must select a routine for each component.
 */

GLOBAL(void)
_jinit_downsampler(j_compress_ptr cinfo)
{
  my_downsample_ptr downsample;
  int ci;
  jpeg_component_info *compptr;

  if (cinfo->data_precision != BITS_IN_JSAMPLE)
    ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);

  downsample = (my_downsample_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(my_downsampler));
  cinfo->downsample = (struct jpeg_downsampler *)downsample;
  downsample->pub.start_pass = start_pass_downsample;
  downsample->pub._downsample = sep_downsample;
  downsample->pub.need_context_rows = FALSE;

  /* Verify we can handle the sampling factors, and set up method pointers */
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    if (compptr->h_samp_factor == cinfo->max_h_samp_factor &&
        compptr->v_samp_factor == cinfo->max_v_samp_factor) {
        downsample->methods[ci] = fullsize_downsample;
    } else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
               compptr->v_samp_factor == cinfo->max_v_samp_factor) {
#ifdef WITH_SIMD
      if (jsimd_can_h2v1_downsample())
        downsample->methods[ci] = jsimd_h2v1_downsample;
      else
#endif
        downsample->methods[ci] = h2v1_downsample;
    } else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
               compptr->v_samp_factor * 2 == cinfo->max_v_samp_factor) {
      {
#ifdef WITH_SIMD
        if (jsimd_can_h2v2_downsample())
          downsample->methods[ci] = jsimd_h2v2_downsample;
        else
#endif
          downsample->methods[ci] = h2v2_downsample;
      }
    } else if ((cinfo->max_h_samp_factor % compptr->h_samp_factor) == 0 &&
               (cinfo->max_v_samp_factor % compptr->v_samp_factor) == 0) {
      downsample->methods[ci] = int_downsample;
    } else
      ERREXIT(cinfo, JERR_FRACT_SAMPLE_NOTIMPL);
  }
}
