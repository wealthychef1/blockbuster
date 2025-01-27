/* db.h -- compression database

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



/*************************************************************************
//
**************************************************************************/

#if defined(HAVE_LZO1_H)
{ "LZO1-1", M_LZO1, LZO1_MEM_COMPRESS, LZO1_MEM_DECOMPRESS,
  lzo1_compress,                0,
  lzo1_decompress,              0,
  0,                            0,
  0,                            0,
  0,                            0 },
#if !defined(LZO_99_UNSUPPORTED)
{ "LZO1-99", M_LZO1_99, LZO1_99_MEM_COMPRESS, LZO1_MEM_DECOMPRESS,
  lzo1_99_compress,             0,
  lzo1_decompress,              0,
  0,                            0,
  0,                            0,
  0,                            0 },
#endif
#endif

#if defined(HAVE_LZO1A_H)
{ "LZO1A-1", M_LZO1A, LZO1A_MEM_COMPRESS, LZO1A_MEM_DECOMPRESS,
  lzo1a_compress,               0,
  lzo1a_decompress,             0,
  0,                            0,
  0,                            0,
  0,                            0 },
#if !defined(LZO_99_UNSUPPORTED)
{ "LZO1A-99", M_LZO1A_99, LZO1A_99_MEM_COMPRESS, LZO1A_MEM_DECOMPRESS,
  lzo1a_99_compress,            0,
  lzo1a_decompress,             0,
  0,                            0,
  0,                            0,
  0,                            0 },
#endif
#endif

#if defined(HAVE_LZO1B_H)
{ "LZO1B-1", M_LZO1B_1, LZO1B_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_1_compress,             0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1B-2", M_LZO1B_2, LZO1B_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_2_compress,             0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1B-3", M_LZO1B_3, LZO1B_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_3_compress,             0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1B-4", M_LZO1B_4, LZO1B_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_4_compress,             0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1B-5", M_LZO1B_5, LZO1B_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_5_compress,             0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1B-6", M_LZO1B_6, LZO1B_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_6_compress,             0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1B-7", M_LZO1B_7, LZO1B_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_7_compress,             0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1B-8", M_LZO1B_8, LZO1B_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_8_compress,             0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1B-9", M_LZO1B_9, LZO1B_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_9_compress,             0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
#if !defined(LZO_99_UNSUPPORTED)
{ "LZO1B-99", M_LZO1B_99, LZO1B_99_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_99_compress,            0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
#endif
#if !defined(LZO_999_UNSUPPORTED)
{ "LZO1B-999", M_LZO1B_999, LZO1B_999_MEM_COMPRESS, LZO1B_MEM_DECOMPRESS,
  lzo1b_999_compress,           0,
  lzo1b_decompress,             lzo1b_decompress_safe,
  lzo1b_decompress_asm,         lzo1b_decompress_asm_safe,
  lzo1b_decompress_asm_fast,    lzo1b_decompress_asm_fast_safe,
  0,                            0 },
#endif
#endif

#if defined(HAVE_LZO1C_H)
{ "LZO1C-1", M_LZO1C_1, LZO1C_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_1_compress,             0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1C-2", M_LZO1C_2, LZO1C_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_2_compress,             0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1C-3", M_LZO1C_3, LZO1C_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_3_compress,             0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1C-4", M_LZO1C_4, LZO1C_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_4_compress,             0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1C-5", M_LZO1C_5, LZO1C_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_5_compress,             0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1C-6", M_LZO1C_6, LZO1C_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_6_compress,             0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1C-7", M_LZO1C_7, LZO1C_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_7_compress,             0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1C-8", M_LZO1C_8, LZO1C_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_8_compress,             0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1C-9", M_LZO1C_9, LZO1C_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_9_compress,             0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
#if !defined(LZO_99_UNSUPPORTED)
{ "LZO1C-99", M_LZO1C_99, LZO1C_99_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_99_compress,            0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
#endif
#if !defined(LZO_999_UNSUPPORTED)
{ "LZO1C-999", M_LZO1C_999, LZO1C_999_MEM_COMPRESS, LZO1C_MEM_DECOMPRESS,
  lzo1c_999_compress,           0,
  lzo1c_decompress,             lzo1c_decompress_safe,
  lzo1c_decompress_asm,         lzo1c_decompress_asm_safe,
  lzo1c_decompress_asm_fast,    lzo1c_decompress_asm_fast_safe,
  0,                            0 },
#endif
#endif

#if defined(HAVE_LZO1F_H)
{ "LZO1F-1", M_LZO1F_1, LZO1F_MEM_COMPRESS, LZO1F_MEM_DECOMPRESS,
  lzo1f_1_compress,             0,
  lzo1f_decompress,             lzo1f_decompress_safe,
  lzo1f_decompress_asm,         lzo1f_decompress_asm_safe,
  lzo1f_decompress_asm_fast,    lzo1f_decompress_asm_fast_safe,
  0,                            0 },
#if !defined(LZO_999_UNSUPPORTED)
{ "LZO1F-999", M_LZO1F_999, LZO1F_999_MEM_COMPRESS, LZO1F_MEM_DECOMPRESS,
  lzo1f_999_compress,           0,
  lzo1f_decompress,             lzo1f_decompress_safe,
  lzo1f_decompress_asm,         lzo1f_decompress_asm_safe,
  lzo1f_decompress_asm_fast,    lzo1f_decompress_asm_fast_safe,
  0,                            0 },
#endif
#endif

#if defined(HAVE_LZO1X_H)
{ "LZO1X-1", M_LZO1X_1, LZO1X_1_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_1_compress,             lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            0 },
#if 0
{ "LZO1XT-1", M_LZO1XT_1, LZO1XT_1_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1xt_1_compress,            lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            0 },
#endif
{ "LZO1X-1(11)", M_LZO1X_1_11, LZO1X_1_11_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_1_11_compress,          lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1X-1(12)", M_LZO1X_1_12, LZO1X_1_12_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_1_12_compress,          lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            0 },
{ "LZO1X-1(15)", M_LZO1X_1_15, LZO1X_1_15_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_1_15_compress,          lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            0 },
#if !defined(LZO_999_UNSUPPORTED)
{ "LZO1X-999", M_LZO1X_999, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_compress,           lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  lzo1x_999_compress_dict,      lzo1x_decompress_dict_safe },
#endif
#if !defined(LZO_999_UNSUPPORTED)
{ "LZO1X-999/1", 9721, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_1_compress,         lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            lzo1x_decompress_dict_safe },
{ "LZO1X-999/2", 9722, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_2_compress,         lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            lzo1x_decompress_dict_safe },
{ "LZO1X-999/3", 9723, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_3_compress,         lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            lzo1x_decompress_dict_safe },
{ "LZO1X-999/4", 9724, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_4_compress,         lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            lzo1x_decompress_dict_safe },
{ "LZO1X-999/5", 9725, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_5_compress,         lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            lzo1x_decompress_dict_safe },
{ "LZO1X-999/6", 9726, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_6_compress,         lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            lzo1x_decompress_dict_safe },
{ "LZO1X-999/7", 9727, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_7_compress,         lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            lzo1x_decompress_dict_safe },
{ "LZO1X-999/8", 9728, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_8_compress,         lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            lzo1x_decompress_dict_safe },
{ "LZO1X-999/9", 9729, LZO1X_999_MEM_COMPRESS, LZO1X_MEM_DECOMPRESS,
  lzo1x_999_9_compress,         lzo1x_optimize,
  lzo1x_decompress,             lzo1x_decompress_safe,
  lzo1x_decompress_asm,         lzo1x_decompress_asm_safe,
  lzo1x_decompress_asm_fast,    lzo1x_decompress_asm_fast_safe,
  0,                            lzo1x_decompress_dict_safe },
#endif
#endif

#if defined(HAVE_LZO1Y_H)
{ "LZO1Y-1", M_LZO1Y_1, LZO1Y_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_1_compress,             lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
#if !defined(LZO_999_UNSUPPORTED)
{ "LZO1Y-999", M_LZO1Y_999, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_compress,           lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  lzo1y_999_compress_dict,      lzo1y_decompress_dict_safe },
#endif
#if !defined(LZO_999_UNSUPPORTED)
{ "LZO1Y-999/1", 9821, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_1_compress,         lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
{ "LZO1Y-999/2", 9822, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_2_compress,         lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
{ "LZO1Y-999/3", 9823, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_3_compress,         lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
{ "LZO1Y-999/4", 9824, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_4_compress,         lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
{ "LZO1Y-999/5", 9825, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_5_compress,         lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
{ "LZO1Y-999/6", 9826, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_6_compress,         lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
{ "LZO1Y-999/7", 9827, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_7_compress,         lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
{ "LZO1Y-999/8", 9828, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_8_compress,         lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
{ "LZO1Y-999/9", 9829, LZO1Y_999_MEM_COMPRESS, LZO1Y_MEM_DECOMPRESS,
  lzo1y_999_9_compress,         lzo1y_optimize,
  lzo1y_decompress,             lzo1y_decompress_safe,
  lzo1y_decompress_asm,         lzo1y_decompress_asm_safe,
  lzo1y_decompress_asm_fast,    lzo1y_decompress_asm_fast_safe,
  0,                            lzo1y_decompress_dict_safe },
#endif
#endif

#if defined(HAVE_LZO1Z_H)
#if !defined(LZO_999_UNSUPPORTED)
{ "LZO1Z-999", M_LZO1Z_999, LZO1Z_999_MEM_COMPRESS, LZO1Z_MEM_DECOMPRESS,
  lzo1z_999_compress,           0,
  lzo1z_decompress,             lzo1z_decompress_safe,
  0,                            0,
  0,                            0,
  lzo1z_999_compress_dict,      lzo1z_decompress_dict_safe },
#endif
#endif

#if defined(HAVE_LZO2A_H)
#if !defined(LZO_999_UNSUPPORTED)
{ "LZO2A-999", M_LZO2A_999, LZO2A_999_MEM_COMPRESS, LZO2A_MEM_DECOMPRESS,
  lzo2a_999_compress,           0,
  lzo2a_decompress,             lzo2a_decompress_safe,
  0,                            0,
  0,                            0,
  0,                            0 },
#endif
#endif

#if defined(HAVE_ZLIB_H)
{ "zlib-8/1", M_ZLIB_8_1, ZLIB_MEM_COMPRESS, ZLIB_MEM_DECOMPRESS,
                                zlib_8_1_compress, 0,
                                zlib_decompress, 0, 0, 0, 0, 0, 0, 0 },
{ "zlib-8/2", M_ZLIB_8_2, ZLIB_MEM_COMPRESS, ZLIB_MEM_DECOMPRESS,
                                zlib_8_2_compress, 0,
                                zlib_decompress, 0, 0, 0, 0, 0, 0, 0 },
{ "zlib-8/3", M_ZLIB_8_3, ZLIB_MEM_COMPRESS, ZLIB_MEM_DECOMPRESS,
                                zlib_8_3_compress, 0,
                                zlib_decompress, 0, 0, 0, 0, 0, 0, 0 },
{ "zlib-8/4", M_ZLIB_8_4, ZLIB_MEM_COMPRESS, ZLIB_MEM_DECOMPRESS,
                                zlib_8_4_compress, 0,
                                zlib_decompress, 0, 0, 0, 0, 0, 0, 0 },
{ "zlib-8/5", M_ZLIB_8_5, ZLIB_MEM_COMPRESS, ZLIB_MEM_DECOMPRESS,
                                zlib_8_5_compress, 0,
                                zlib_decompress, 0, 0, 0, 0, 0, 0, 0 },
{ "zlib-8/6", M_ZLIB_8_6, ZLIB_MEM_COMPRESS, ZLIB_MEM_DECOMPRESS,
                                zlib_8_6_compress, 0,
                                zlib_decompress, 0, 0, 0, 0, 0, 0, 0 },
{ "zlib-8/7", M_ZLIB_8_7, ZLIB_MEM_COMPRESS, ZLIB_MEM_DECOMPRESS,
                                zlib_8_7_compress, 0,
                                zlib_decompress, 0, 0, 0, 0, 0, 0, 0 },
{ "zlib-8/8", M_ZLIB_8_8, ZLIB_MEM_COMPRESS, ZLIB_MEM_DECOMPRESS,
                                zlib_8_8_compress, 0,
                                zlib_decompress, 0, 0, 0, 0, 0, 0, 0 },
{ "zlib-8/9", M_ZLIB_8_9, ZLIB_MEM_COMPRESS, ZLIB_MEM_DECOMPRESS,
                                zlib_8_9_compress, 0,
                                zlib_decompress, 0, 0, 0, 0, 0, 0, 0 },
#endif

{ "memcpy()", M_MEMCPY, 0, 0,   memcpy_x_compress, 0,
                                memcpy_x_compress, 0, 0, 0, 0, 0, 0, 0 },
{ "memset()", M_MEMSET, 0, 0,   memset_x_compress, 0,
                                memset_x_compress, 0, 0, 0, 0, 0, 0, 0 },
{ "adler32()", M_ADLER32, 0, 0, adler32_x_compress, 0,
                                adler32_x_compress, 0, 0, 0, 0, 0, 0, 0 },
{ "crc32()", M_CRC32, 0, 0,     crc32_x_compress, 0,
                                crc32_x_compress, 0, 0, 0, 0, 0, 0, 0 },
#if defined(HAVE_ZLIB_H)
{ "z_adler32()", M_Z_ADLER32, 0, 0, zlib_adler32_x_compress, 0,
                                zlib_adler32_x_compress, 0, 0, 0, 0, 0, 0, 0 },
{ "z_crc32()", M_Z_CRC32, 0, 0, zlib_crc32_x_compress, 0,
                                zlib_crc32_x_compress, 0, 0, 0, 0, 0, 0, 0 },
#endif

#if defined(MFX)
#  include "maint/t_db.ch"
#endif



/*
vi:ts=4
*/

