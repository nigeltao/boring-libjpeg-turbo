/*
 * jcprepct.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * Lossless JPEG Modifications:
 * Copyright (C) 1999, Ken Murchison.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains the compression preprocessing controller.
 * This controller manages the color conversion, downsampling,
 * and edge expansion steps.
 *
 * Most of the complexity here is associated with buffering input rows
 * as required by the downsampler.  See the comments at the head of
 * jcsample.c for the downsampler's needs.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jsamplecomp.h"


/*
 * For the simple (no-context-row) case, we just need to buffer one
 * row group's worth of pixels for the downsampling step.  At the bottom of
 * the image, we pad to a full row group by replicating the last pixel row.
 * The downsampler's last output row is then replicated if needed to pad
 * out to a full iMCU row.
 *
 * When providing context rows, we must buffer three row groups' worth of
 * pixels.  Three row groups are physically allocated, but the row pointer
 * arrays are made five row groups high, with the extra pointers above and
 * below "wrapping around" to point to the last and first real row groups.
 * This allows the downsampler to access the proper context rows.
 * At the top and bottom of the image, we create dummy context rows by
 * copying the first or last real pixel row.  This copying could be avoided
 * by pointer hacking as is done in jdmainct.c, but it doesn't seem worth the
 * trouble on the compression side.
 */


/* Private buffer controller object */

typedef struct {
  struct jpeg_c_prep_controller pub; /* public fields */

  /* Downsampling input buffer.  This buffer holds color-converted data
   * until we have enough to do a downsample step.
   */
  _JSAMPARRAY color_buf[MAX_COMPONENTS];

  JDIMENSION rows_to_go;        /* counts rows remaining in source image */
  int next_buf_row;             /* index of next row to store in color_buf */

} my_prep_controller;

typedef my_prep_controller *my_prep_ptr;


/*
 * Initialize for a processing pass.
 */

METHODDEF(void)
start_pass_prep(j_compress_ptr cinfo, J_BUF_MODE pass_mode)
{
  my_prep_ptr prep = (my_prep_ptr)cinfo->prep;

  if (pass_mode != JBUF_PASS_THRU)
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);

  /* Initialize total-height counter for detecting bottom of image */
  prep->rows_to_go = cinfo->image_height;
  /* Mark the conversion buffer empty */
  prep->next_buf_row = 0;
}


/*
 * Expand an image vertically from height input_rows to height output_rows,
 * by duplicating the bottom row.
 */

LOCAL(void)
expand_bottom_edge(_JSAMPARRAY image_data, JDIMENSION num_cols, int input_rows,
                   int output_rows)
{
  register int row;

  for (row = input_rows; row < output_rows; row++) {
    _jcopy_sample_rows(image_data, input_rows - 1, image_data, row, 1,
                       num_cols);
  }
}


/*
 * Process some data in the simple no-context case.
 *
 * Preprocessor output data is counted in "row groups".  A row group
 * is defined to be v_samp_factor sample rows of each component.
 * Downsampling will produce this much data from each max_v_samp_factor
 * input rows.
 */

METHODDEF(void)
pre_process_data(j_compress_ptr cinfo, _JSAMPARRAY input_buf,
                 JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail,
                 _JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr,
                 JDIMENSION out_row_groups_avail)
{
  my_prep_ptr prep = (my_prep_ptr)cinfo->prep;
  int numrows, ci;
  JDIMENSION inrows;
  jpeg_component_info *compptr;

  while (*in_row_ctr < in_rows_avail &&
         *out_row_group_ctr < out_row_groups_avail) {
    /* Do color conversion to fill the conversion buffer. */
    inrows = in_rows_avail - *in_row_ctr;
    numrows = cinfo->max_v_samp_factor - prep->next_buf_row;
    numrows = (int)MIN((JDIMENSION)numrows, inrows);
    (*cinfo->cconvert->_color_convert) (cinfo, input_buf + *in_row_ctr,
                                        prep->color_buf,
                                        (JDIMENSION)prep->next_buf_row,
                                        numrows);
    *in_row_ctr += numrows;
    prep->next_buf_row += numrows;
    prep->rows_to_go -= numrows;
    /* If at bottom of image, pad to fill the conversion buffer. */
    if (prep->rows_to_go == 0 &&
        prep->next_buf_row < cinfo->max_v_samp_factor) {
      for (ci = 0; ci < cinfo->num_components; ci++) {
        expand_bottom_edge(prep->color_buf[ci], cinfo->image_width,
                           prep->next_buf_row, cinfo->max_v_samp_factor);
      }
      prep->next_buf_row = cinfo->max_v_samp_factor;
    }
    /* If we've filled the conversion buffer, empty it. */
    if (prep->next_buf_row == cinfo->max_v_samp_factor) {
      (*cinfo->downsample->_downsample) (cinfo,
                                         prep->color_buf, (JDIMENSION)0,
                                         output_buf, *out_row_group_ctr);
      prep->next_buf_row = 0;
      (*out_row_group_ctr)++;
    }
    /* If at bottom of image, pad the output to a full iMCU height.
     * Note we assume the caller is providing a one-iMCU-height output buffer!
     */
    if (prep->rows_to_go == 0 && *out_row_group_ctr < out_row_groups_avail) {
      for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
           ci++, compptr++) {
        expand_bottom_edge(output_buf[ci],
                           compptr->width_in_blocks * DCTSIZE,
                           (int)(*out_row_group_ctr * compptr->v_samp_factor),
                           (int)(out_row_groups_avail * compptr->v_samp_factor));
      }
      *out_row_group_ctr = out_row_groups_avail;
      break;                    /* can exit outer loop without test */
    }
  }
}


/*
 * Initialize preprocessing controller.
 */

GLOBAL(void)
_jinit_c_prep_controller(j_compress_ptr cinfo, boolean need_full_buffer)
{
  my_prep_ptr prep;
  int ci;
  jpeg_component_info *compptr;

  if (cinfo->data_precision != BITS_IN_JSAMPLE)
    ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);

  if (need_full_buffer)         /* safety check */
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);

  prep = (my_prep_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(my_prep_controller));
  cinfo->prep = (struct jpeg_c_prep_controller *)prep;
  prep->pub.start_pass = start_pass_prep;

  /* Allocate the color conversion buffer.
   * We make the buffer wide enough to allow the downsampler to edge-expand
   * horizontally within the buffer, if it so chooses.
   */
  if (cinfo->downsample->need_context_rows) {
    ERREXIT(cinfo, JERR_NOT_COMPILED);
  } else {
    /* No context, just make it tall enough for one row group */
    prep->pub._pre_process_data = pre_process_data;
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
         ci++, compptr++) {
      prep->color_buf[ci] = (_JSAMPARRAY)(*cinfo->mem->alloc_sarray)
        ((j_common_ptr)cinfo, JPOOL_IMAGE,
         (JDIMENSION)(((long)compptr->width_in_blocks * DCTSIZE *
                       cinfo->max_h_samp_factor) / compptr->h_samp_factor),
         (JDIMENSION)cinfo->max_v_samp_factor);
    }
  }
}
