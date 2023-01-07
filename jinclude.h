/*
 * jinclude.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1994, Thomas G. Lane.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2022, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file exists to provide a single place to fix any problems with
 * including the wrong system include files.  (Common problems are taken
 * care of by the standard jconfig symbols, but on really weird systems
 * you may have to edit this file.)
 *
 * NOTE: this file is NOT intended to be included by applications using the
 * JPEG library.  Most applications need only include jpeglib.h.
 */

#ifndef __JINCLUDE_H__
#define __JINCLUDE_H__

/* Include auto-config file to find out which system include files we need. */

#include "jconfig.h"            /* auto configuration options */
#include "jconfigint.h"
#define JCONFIG_INCLUDED        /* so that jpeglib.h doesn't do it again */

/*
 * Note that the core JPEG library does not require <stdio.h>;
 * only the default error handler and data source/destination modules do.
 * But we must pull it in because of the references to FILE in jpeglib.h.
 * You can remove those references if you want to compile without <stdio.h>.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * These macros/inline functions facilitate using Microsoft's "safe string"
 * functions with Visual Studio builds without the need to scatter #ifdefs
 * throughout the code base.
 */


#ifdef _MSC_VER

#define SNPRINTF(str, n, format, ...) \
  _snprintf_s(str, n, _TRUNCATE, format, ##__VA_ARGS__)

#else

#define SNPRINTF  snprintf

#endif


#endif /* JINCLUDE_H */
