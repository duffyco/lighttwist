/*
 * This file is part of IMGU.
 * 
 * @Copyright 2008 Université de Montréal, Laboratoire Vision3D
 *   Sébastien Roy (roys@iro.umontreal.ca)
 *   Vincent Chapdelaine-Couture (chapdelv@iro.umontreal.ca)
 *   Lucie Bélanger
 *   Jamil Draréni 
 *
 * IMGU is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * IMGU is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with IMGU.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Imgu Image Library
 *
 *
 * This is a 16-bit image library!
 *
 * @version $Revision: 1.0 $
 *
 * @date $Date: 2007/12/26 23:27:20 $
 *
 * $Id: doxygen-howto.html,v 1.5 2005/04/14 14:16:20 bv Exp $
 *
 */

/**
 * @defgroup imgugeneral IMGU Image Library
 *
 * Image variables are pointers to the imgu structure and they MUST always be initialized to NULL.
 *
 * An imgu structure can be allocated using imguAllocate(), imguLoad() or imguLoadMulti().
 * Nearly all IMGU functions working on image data have the form
 * imguXXXXX(imgu **dest,imgu *src, ...), where @p dest refers to the address
 * of an image pointer.
 * The @p src image has to be allocated beforehand, but the @p dest image is allocated
 * on the fly. Furthermore, @p src and @p dest can point to the same image structure,
 * with the exception of the functions imguConvertToComplexFromColor() and
 * imguConvertFromComplexToColor().
 *
 * @attention In gereral, IMGU functions do not copy text information from @p src
 * to @p dest and imguCopyText() must be called to do so.
 * Exceptions where text is indeed copied from @p src to @p dest are imguCopy(), 
 * imguExtractRectangle(), imguSelectChannels(), imguConcat(), imguSplit().
 *
 * To work with complex data, image data can be converted from and to complex data by using
 * imguConvertToComplex(),imguConvertFromComplex(),imguConvertToComplexFromColor(),
 * imguConvertFromComplexToColor(). An empty complex image can also be allocated using
 * imguAllocateComplex().
 *
 * IMGU functions either work only on image data (\ref images) or only on complex data
 * (\ref complex). However, the following functions
 * work on both image and complex data (if applicable): imguCopy(), imguExtractRectangle(), imguSelectChannels(), imguConcat().
 *
 * Here is a sample program.
 *
 * It is highly recommended to reused image pointers as much as possible. Image pointers should only be freed when not used anymore. 
 *
 * @code
 *
 * imgu *Ia,*Ib;
 * Ia=NULL;Ib=NULL;
 *
 * imguLoad(&Ia,"someimage.png",LOAD_16_BITS); //or imguAllocate(&Ia,256,256,3); for instance
 * imguScale(&Ib,Ia,2.0,2.0); // imguScale(&Ia,Ia,2.0,2.0); could also be called
 * imguSave(&Ib,"scaledimage.png",FAST_COMPRESSION,SAVE_16_BITS);
 *
 * imguFree(&Ia);
 * imguFree(&Ib);    
 *
 * @endcode
 *
 */



/**
 * @defgroup apps Applications
 *
 * This section groups many standalone programs that imitate the style og netpbm applications, but
 * for PNG images. Interesting features are:
 *     - works with filenames or standard input/output (i.e. easy to pipe commands)
 *     - works with concatenated images (can process video)
 *
 */


#ifndef IMGU_H
#define IMGU_H

#include <stdio.h>
#include <stdlib.h>

/* #undef HAVE_BMC */
/* #undef HAVE_PT */
#define HAVE_JPEG
#define HAVE_NETPBM
#define HAVE_IMGU_EXTRA
/* #undef HAVE_RQUEUE */
#define HAVE_FFTW3
/* #undef HAVE_PROFILOMETRE */

#define IMGU_DATA "/usr/local/share/imgu"

// we use "" instead of <> to include from the local directory.

#include "param.h"
#include "matrixmath.h"

#include "imgu_core.h"

#include "imgu_canny.h"
#include "imgu_camera.h"
#include "imgu_draw.h"
#include "imgu_interpolate.h"
#include "imgu_interpolate_view.h"
#include "imgu_osg.h"
#include "imgu_matrix.h"
#include "imgu_pyramid.h"
#include "imgu_sift.h"
#include "imgu_stereo.h"
#include "imgu_seam.h"
#include "imgu_args.h"
#include "imgu_complex.h"

#ifdef HAVE_RQUEUE
  #include "imgu_plugin.h"
  #include "imgu_recycle.h"
#endif

#ifdef HAVE_JPEG
	#include "imgu_jpeg.h"
#endif

#ifdef HAVE_NETPBM
	#include "imgu_netpbm.h"
#endif

//
// from this point, we only include if compiling C++
//
#ifdef __cplusplus

	#ifdef HAVE_IMGU_EXTRA
		#include "GPU.h"
	#endif

#endif


#endif
