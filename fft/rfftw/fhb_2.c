/*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Tue May 18 13:55:57 EDT 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -hc2hc-backward 2 */

/*
 * This function contains 8 FP additions, 6 FP multiplications,
 * (or, 6 additions, 4 multiplications, 2 fused multiply/add),
 * 11 stack variables, and 16 memory accesses
 */
static const fftw_real K2_000000000 = FFTW_KONST(+2.000000000000000000000000000000000000000000000);

void fftw_hc2hc_backward_2(fftw_real *A, const fftw_complex *W, int iostride, int m, int dist)
{
     int i;
     fftw_real *X;
     fftw_real *Y;
     X = A;
     Y = A + (2 * iostride);
     {
	  fftw_real tmp11;
	  fftw_real tmp12;
	  ASSERT_ALIGNED_DOUBLE();
	  tmp11 = X[0];
	  tmp12 = X[iostride];
	  X[iostride] = tmp11 - tmp12;
	  X[0] = tmp11 + tmp12;
     }
     X = X + dist;
     Y = Y - dist;
     for (i = 2; i < m; i = i + 2, X = X + dist, Y = Y - dist, W = W + 1) {
	  fftw_real tmp3;
	  fftw_real tmp4;
	  fftw_real tmp8;
	  fftw_real tmp5;
	  fftw_real tmp6;
	  fftw_real tmp10;
	  fftw_real tmp7;
	  fftw_real tmp9;
	  ASSERT_ALIGNED_DOUBLE();
	  tmp3 = X[0];
	  tmp4 = Y[-iostride];
	  tmp8 = tmp3 - tmp4;
	  tmp5 = Y[0];
	  tmp6 = X[iostride];
	  tmp10 = tmp5 + tmp6;
	  X[0] = tmp3 + tmp4;
	  Y[-iostride] = tmp5 - tmp6;
	  tmp7 = c_re(W[0]);
	  tmp9 = c_im(W[0]);
	  X[iostride] = (tmp7 * tmp8) + (tmp9 * tmp10);
	  Y[0] = (tmp7 * tmp10) - (tmp9 * tmp8);
     }
     if (i == m) {
	  fftw_real tmp1;
	  fftw_real tmp2;
	  ASSERT_ALIGNED_DOUBLE();
	  tmp1 = X[0];
	  X[0] = K2_000000000 * tmp1;
	  tmp2 = Y[0];
	  X[iostride] = -(K2_000000000 * tmp2);
     }
}

static const int twiddle_order[] =
{1};
fftw_codelet_desc fftw_hc2hc_backward_2_desc =
{
     "fftw_hc2hc_backward_2",
     (void (*)()) fftw_hc2hc_backward_2,
     2,
     FFTW_BACKWARD,
     FFTW_HC2HC,
     58,
     1,
     twiddle_order,
};
