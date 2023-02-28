/*
 * jdmaster.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1995, Thomas G. Lane.
 * boring-libjpeg-turbo Modifications:
 * Copyright (C) 2023, Nigel Tao.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains the master control structure for the JPEG decompressor.
 */

/* Private state */

typedef struct {
  struct jpeg_decomp_master pub; /* public fields */

  int pass_number;              /* # of passes completed */

  boolean notboring_using_merged_upsample;

  void *notboring_quantizer_1pass;
  void *notboring_quantizer_2pass;
} my_decomp_master;

typedef my_decomp_master *my_master_ptr;
