/* lzo1y_1.c -- LZO1Y-1 compression

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#define LZO_NEED_DICT_H
#define D_BITS			14
#define D_INDEX1(d,p)		d = DX3(p,5,5,6); d += d >> 5; d = DM(d)
#define D_INDEX2(d,p)		d = (d & (D_MASK & 0x7ff)) ^ (D_HIGH | 0x1f)

#include "config1y.h"

#define DO_COMPRESS		lzo1y_1_compress

#include "lzo1x_c.ch"
