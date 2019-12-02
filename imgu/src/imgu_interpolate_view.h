#ifndef IMGU_INTERPOLATE_VIEW_H
#define IMGU_INTERPOLATE_VIEW_H

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

#include "imgu.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup images
 * @{
 *
 * @name Stereo algorithms
 * @{
 */


  int imguForwardMapping(imgu **Idest,imgu *dmA,imgu *dmB,imgu *IA,imgu *IB,double alpha);
  int imguBackwardMapping(imgu **Idest,imgu *dmA,imgu *dmB,imgu *IA,imgu *IB,double alpha);

/*@}*/
/*@}*/

#ifdef __cplusplus
}
#endif


#endif
