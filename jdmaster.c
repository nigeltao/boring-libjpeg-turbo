/*
 * jdmaster.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Modified 2002-2009 by Guido Vollbeding.
 * Lossless JPEG Modifications:
 * Copyright (C) 1999, Ken Murchison.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2009-2011, 2016, 2019, 2022, D. R. Commander.
 * Copyright (C) 2013, Linaro Limited.
 * Copyright (C) 2015, Google, Inc.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains master control logic for the JPEG decompressor.
 * These routines are concerned with selecting the modules to be executed
 * and with determining the number of passes and the work to be done in each
 * pass.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jpegapicomp.h"
#include "jdmaster.h"


/*
 * Compute output image dimensions and related values.
 * NOTE: this is exported for possible use by application.
 * Hence it mustn't do anything that can't be done twice.
 */

#if JPEG_LIB_VERSION >= 80
GLOBAL(void)
#else
LOCAL(void)
#endif
jpeg_core_output_dimensions(j_decompress_ptr cinfo)
/* Do computations that are needed before master selection phase.
 * This function is used for transcoding and full decompression.
 */
{
  {
    /* Hardwire it to "no scaling" */
    cinfo->output_width = cinfo->image_width;
    cinfo->output_height = cinfo->image_height;
  }
}


/*
 * Compute output image dimensions and related values.
 * NOTE: this is exported for possible use by application.
 * Hence it mustn't do anything that can't be done twice.
 * Also note that it may be called before the master module is initialized!
 */

GLOBAL(void)
jpeg_calc_output_dimensions(j_decompress_ptr cinfo)
/* Do computations that are needed before master selection phase */
{
  /* Prevent application from calling me at wrong times */
  if (cinfo->global_state != DSTATE_READY)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  /* Compute core output image dimensions and DCT scaling choices. */
  jpeg_core_output_dimensions(cinfo);

  {
    /* Hardwire it to "no scaling" */
    cinfo->output_width = cinfo->image_width;
    cinfo->output_height = cinfo->image_height;
  }

  /* Report number of components in selected colorspace. */
  /* Probably this should be in the color conversion module... */
  switch (cinfo->out_color_space) {
  case JCS_GRAYSCALE:
    cinfo->out_color_components = 1;
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
    cinfo->out_color_components = rgb_pixelsize[cinfo->out_color_space];
    break;
  case JCS_YCbCr:
    cinfo->out_color_components = 3;
    break;
  case JCS_CMYK:
  case JCS_YCCK:
    cinfo->out_color_components = 4;
    break;
  default:                      /* else must be same colorspace as in file */
    cinfo->out_color_components = cinfo->num_components;
    break;
  }
  cinfo->output_components = cinfo->out_color_components;

  cinfo->rec_outbuf_height = 1;
}


/*
 * Several decompression processes need to range-limit values to the range
 * 0..MAXJSAMPLE; the input value may fall somewhat outside this range
 * due to noise introduced by quantization, roundoff error, etc.  These
 * processes are inner loops and need to be as fast as possible.  On most
 * machines, particularly CPUs with pipelines or instruction prefetch,
 * a (subscript-check-less) C table lookup
 *              x = sample_range_limit[x];
 * is faster than explicit tests
 *              if (x < 0)  x = 0;
 *              else if (x > MAXJSAMPLE)  x = MAXJSAMPLE;
 * These processes all use a common table prepared by the routine below.
 *
 * For most steps we can mathematically guarantee that the initial value
 * of x is within MAXJSAMPLE+1 of the legal range, so a table running from
 * -(MAXJSAMPLE+1) to 2*MAXJSAMPLE+1 is sufficient.  But for the initial
 * limiting step (just after the IDCT), a wildly out-of-range value is
 * possible if the input data is corrupt.  To avoid any chance of indexing
 * off the end of memory and getting a bad-pointer trap, we perform the
 * post-IDCT limiting thus:
 *              x = range_limit[x & MASK];
 * where MASK is 2 bits wider than legal sample data, ie 10 bits for 8-bit
 * samples.  Under normal circumstances this is more than enough range and
 * a correct output will be generated; with bogus input data the mask will
 * cause wraparound, and we will safely generate a bogus-but-in-range output.
 * For the post-IDCT step, we want to convert the data from signed to unsigned
 * representation by adding CENTERJSAMPLE at the same time that we limit it.
 * So the post-IDCT limiting table ends up looking like this:
 *   CENTERJSAMPLE,CENTERJSAMPLE+1,...,MAXJSAMPLE,
 *   MAXJSAMPLE (repeat 2*(MAXJSAMPLE+1)-CENTERJSAMPLE times),
 *   0          (repeat 2*(MAXJSAMPLE+1)-CENTERJSAMPLE times),
 *   0,1,...,CENTERJSAMPLE-1
 * Negative inputs select values from the upper half of the table after
 * masking.
 *
 * We can save some space by overlapping the start of the post-IDCT table
 * with the simpler range limiting table.  The post-IDCT table begins at
 * sample_range_limit + CENTERJSAMPLE.
 */

LOCAL(void)
prepare_range_limit_table(j_decompress_ptr cinfo)
/* Allocate and fill in the sample_range_limit table */
{
  JSAMPLE *table;
  int i;

  if (cinfo->data_precision == 16) {
    ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);
  } else if (cinfo->data_precision == 12) {
    ERREXIT(cinfo, JERR_NOT_COMPILED);
  } else {
    table = (JSAMPLE *)
      (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                  (5 * (MAXJSAMPLE + 1) + CENTERJSAMPLE) * sizeof(JSAMPLE));
    table += (MAXJSAMPLE + 1);  /* allow negative subscripts of simple table */
    cinfo->sample_range_limit = table;
    /* First segment of "simple" table: limit[x] = 0 for x < 0 */
    memset(table - (MAXJSAMPLE + 1), 0, (MAXJSAMPLE + 1) * sizeof(JSAMPLE));
    /* Main part of "simple" table: limit[x] = x */
    for (i = 0; i <= MAXJSAMPLE; i++)
      table[i] = (JSAMPLE)i;
    table += CENTERJSAMPLE;     /* Point to where post-IDCT table starts */
    /* End of simple table, rest of first half of post-IDCT table */
    for (i = CENTERJSAMPLE; i < 2 * (MAXJSAMPLE + 1); i++)
      table[i] = MAXJSAMPLE;
    /* Second half of post-IDCT table */
    memset(table + (2 * (MAXJSAMPLE + 1)), 0,
           (2 * (MAXJSAMPLE + 1) - CENTERJSAMPLE) * sizeof(JSAMPLE));
    memcpy(table + (4 * (MAXJSAMPLE + 1) - CENTERJSAMPLE),
           cinfo->sample_range_limit, CENTERJSAMPLE * sizeof(JSAMPLE));
  }
}


/*
 * Master selection of decompression modules.
 * This is done once at jpeg_start_decompress time.  We determine
 * which modules will be used and give them appropriate initialization calls.
 * We also initialize the decompressor input side to begin consuming data.
 *
 * Since jpeg_read_header has finished, we know what is in the SOF
 * and (first) SOS markers.  We also have all the application parameter
 * settings.
 */

LOCAL(void)
master_selection(j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr)cinfo->master;
  boolean use_c_buffer;
  long samplesperrow;
  JDIMENSION jd_samplesperrow;

  /* Initialize dimensions and other stuff */
  jpeg_calc_output_dimensions(cinfo);
  prepare_range_limit_table(cinfo);

  /* Width of an output scanline must be representable as JDIMENSION. */
  samplesperrow = (long)cinfo->output_width *
                  (long)cinfo->out_color_components;
  jd_samplesperrow = (JDIMENSION)samplesperrow;
  if ((long)jd_samplesperrow != samplesperrow)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);

  /* Initialize my private state */
  master->pass_number = 0;

  /* Post-processing: in particular, color conversion first */
  if (!cinfo->raw_data_out) {
    if (BORING_ALWAYS_TRUE) {
      if (cinfo->data_precision == 16) {
        ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);
      } else if (cinfo->data_precision == 12) {
        ERREXIT(cinfo, JERR_NOT_COMPILED);
      } else {
        jinit_color_deconverter(cinfo);
        jinit_upsampler(cinfo);
      }
    }
    if (cinfo->data_precision == 16)
      ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);
    else if (cinfo->data_precision == 12)
      ERREXIT(cinfo, JERR_NOT_COMPILED);
    else
      jinit_d_post_controller(cinfo, FALSE);
  }

  if (BORING_ALWAYS_TRUE) {
    if (cinfo->data_precision == 16)
      ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);
    /* Inverse DCT */
    if (cinfo->data_precision == 12)
      ERREXIT(cinfo, JERR_NOT_COMPILED);
    else
      jinit_inverse_dct(cinfo);
    /* Entropy decoding: either Huffman or arithmetic coding. */
    if (BORING_ALWAYS_TRUE) {
      if (cinfo->progressive_mode) {
#ifdef D_PROGRESSIVE_SUPPORTED
        jinit_phuff_decoder(cinfo);
#else
        ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
      } else
        jinit_huff_decoder(cinfo);
    }

    /* Initialize principal buffer controllers. */
    use_c_buffer = cinfo->inputctl->has_multiple_scans ||
                   cinfo->buffered_image;
    if (cinfo->data_precision == 12)
      ERREXIT(cinfo, JERR_NOT_COMPILED);
    else
      jinit_d_coef_controller(cinfo, use_c_buffer);
  }

  if (!cinfo->raw_data_out) {
    if (cinfo->data_precision == 16)
      ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);
    else if (cinfo->data_precision == 12)
      ERREXIT(cinfo, JERR_NOT_COMPILED);
    else
      jinit_d_main_controller(cinfo, FALSE /* never need full buffer here */);
  }

  /* We can now tell the memory manager to allocate virtual arrays. */
  (*cinfo->mem->realize_virt_arrays) ((j_common_ptr)cinfo);

  /* Initialize input side of decompressor to consume first scan. */
  (*cinfo->inputctl->start_input_pass) (cinfo);

  /* Set the first and last iMCU columns to decompress from single-scan images.
   * By default, decompress all of the iMCU columns.
   */
  cinfo->master->first_iMCU_col = 0;
  cinfo->master->last_iMCU_col = cinfo->MCUs_per_row - 1;
  cinfo->master->last_good_iMCU_row = 0;

#ifdef D_MULTISCAN_FILES_SUPPORTED
  /* If jpeg_start_decompress will read the whole file, initialize
   * progress monitoring appropriately.  The input step is counted
   * as one pass.
   */
  if (cinfo->progress != NULL && !cinfo->buffered_image &&
      cinfo->inputctl->has_multiple_scans) {
    int nscans;
    /* Estimate number of scans to set pass_limit. */
    if (cinfo->progressive_mode) {
      /* Arbitrarily estimate 2 interleaved DC scans + 3 AC scans/component. */
      nscans = 2 + 3 * cinfo->num_components;
    } else {
      /* For a nonprogressive multiscan file, estimate 1 scan per component. */
      nscans = cinfo->num_components;
    }
    cinfo->progress->pass_counter = 0L;
    cinfo->progress->pass_limit = (long)cinfo->total_iMCU_rows * nscans;
    cinfo->progress->completed_passes = 0;
    cinfo->progress->total_passes = 2;
    /* Count the input pass as done */
    master->pass_number++;
  }
#endif /* D_MULTISCAN_FILES_SUPPORTED */
}


/*
 * Per-pass setup.
 * This is called at the beginning of each output pass.  We determine which
 * modules will be active during this pass and give them appropriate
 * start_pass calls.  We also set is_dummy_pass to indicate whether this
 * is a "real" output pass or a dummy pass for color quantization.
 * (In the latter case, jdapistd.c will crank the pass to completion.)
 */

METHODDEF(void)
prepare_for_output_pass(j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr)cinfo->master;

  if (BORING_ALWAYS_TRUE) {
    (*cinfo->idct->start_pass) (cinfo);
    (*cinfo->coef->start_output_pass) (cinfo);
    if (!cinfo->raw_data_out) {
      (*cinfo->cconvert->start_pass) (cinfo);
      (*cinfo->upsample->start_pass) (cinfo);
      (*cinfo->post->start_pass) (cinfo, JBUF_PASS_THRU);
      (*cinfo->main->start_pass) (cinfo, JBUF_PASS_THRU);
    }
  }

  /* Set up progress monitor's pass info if present */
  if (cinfo->progress != NULL) {
    cinfo->progress->completed_passes = master->pass_number;
    cinfo->progress->total_passes = master->pass_number + 1;
    /* In buffered-image mode, we assume one more output pass if EOI not
     * yet reached, but no more passes if EOI has been reached.
     */
    if (cinfo->buffered_image && !cinfo->inputctl->eoi_reached) {
      cinfo->progress->total_passes += 1;
    }
  }
}


/*
 * Finish up at end of an output pass.
 */

METHODDEF(void)
finish_output_pass(j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr)cinfo->master;

  master->pass_number++;
}


#ifdef D_MULTISCAN_FILES_SUPPORTED

/*
 * Switch to a new external colormap between output passes.
 */

GLOBAL(void)
jpeg_new_colormap(j_decompress_ptr cinfo)
{
  ERREXIT(cinfo, JERR_NOT_COMPILED);
}

#endif /* D_MULTISCAN_FILES_SUPPORTED */


/*
 * Initialize master decompression control and select active modules.
 * This is performed at the start of jpeg_start_decompress.
 */

GLOBAL(void)
jinit_master_decompress(j_decompress_ptr cinfo)
{
  my_master_ptr master = (my_master_ptr)cinfo->master;

  master->pub.prepare_for_output_pass = prepare_for_output_pass;
  master->pub.finish_output_pass = finish_output_pass;

  master->pub.jinit_upsampler_no_alloc = FALSE;

  master_selection(cinfo);
}
