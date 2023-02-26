/*
 * jdpostct.c
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2022, D. R. Commander.
 * It was modified by The libjpeg-turbo Project to include only code relevant
 * to libjpeg-turbo.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains the decompression postprocessing controller.
 * This controller manages the upsampling, color conversion, and color
 * quantization/reduction steps; specifically, it controls the buffering
 * between upsample/color conversion and color quantization/reduction.
 *
 * If no color quantization/reduction is required, then this module has no
 * work to do, and it just hands off to the upsample/color conversion code.
 * An integrated upsample/convert/quantize process would replace this module
 * entirely.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jsamplecomp.h"


/* Private buffer controller object */

typedef struct {
  struct jpeg_d_post_controller pub; /* public fields */
} my_post_controller;

typedef my_post_controller *my_post_ptr;


/*
 * Initialize for a processing pass.
 */

METHODDEF(void)
start_pass_dpost(j_decompress_ptr cinfo, J_BUF_MODE pass_mode)
{
  my_post_ptr post = (my_post_ptr)cinfo->post;

  switch (pass_mode) {
  case JBUF_PASS_THRU:
    if (BORING_ALWAYS_TRUE) {
      /* For single-pass processing without color quantization,
       * I have no work to do; just call the upsampler directly.
       */
      post->pub._post_process_data = cinfo->upsample->_upsample;
    }
    break;
  default:
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    break;
  }
}


/*
 * Initialize postprocessing controller.
 */

GLOBAL(void)
_jinit_d_post_controller(j_decompress_ptr cinfo, boolean need_full_buffer)
{
  my_post_ptr post;

  if (cinfo->data_precision != BITS_IN_JSAMPLE)
    ERREXIT1(cinfo, JERR_BAD_PRECISION, cinfo->data_precision);

  post = (my_post_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(my_post_controller));
  cinfo->post = (struct jpeg_d_post_controller *)post;
  post->pub.start_pass = start_pass_dpost;
}
