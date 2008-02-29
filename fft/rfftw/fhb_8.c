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
/* Generated on Tue May 18 13:56:04 EDT 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -hc2hc-backward 8 */

/*
 * This function contains 108 FP additions, 50 FP multiplications,
 * (or, 90 additions, 32 multiplications, 18 fused multiply/add),
 * 31 stack variables, and 64 memory accesses
 */
static const fftw_real K765366864 = FFTW_KONST(+0.765366864730179543456919968060797733522689125);
static const fftw_real K1_847759065 = FFTW_KONST(+1.847759065022573512256366378793576573644833252);
static const fftw_real K707106781 = FFTW_KONST(+0.707106781186547524400844362104849039284835938);
static const fftw_real K1_414213562 = FFTW_KONST(+1.414213562373095048801688724209698078569671875);
static const fftw_real K2_000000000 = FFTW_KONST(+2.000000000000000000000000000000000000000000000);

void fftw_hc2hc_backward_8(fftw_real *A, const fftw_complex *W, int iostride, int m, int dist)
{
     int i;
     fftw_real *X;
     fftw_real *Y;
     X = A;
     Y = A + (8 * iostride);
     {
	  fftw_real tmp107;
	  fftw_real tmp118;
	  fftw_real tmp105;
	  fftw_real tmp116;
	  fftw_real tmp111;
	  fftw_real tmp120;
	  fftw_real tmp115;
	  fftw_real tmp121;
	  fftw_real tmp108;
	  fftw_real tmp112;
	  ASSERT_ALIGNED_DOUBLE();
	  {
	       fftw_real tmp106;
	       fftw_real tmp117;
	       fftw_real tmp103;
	       fftw_real tmp104;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp106 = X[2 * iostride];
	       tmp107 = K2_000000000 * tmp106;
	       tmp117 = Y[-2 * iostride];
	       tmp118 = K2_000000000 * tmp117;
	       tmp103 = X[0];
	       tmp104 = X[4 * iostride];
	       tmp105 = tmp103 + tmp104;
	       tmp116 = tmp103 - tmp104;
	       {
		    fftw_real tmp109;
		    fftw_real tmp110;
		    fftw_real tmp113;
		    fftw_real tmp114;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp109 = X[iostride];
		    tmp110 = X[3 * iostride];
		    tmp111 = K2_000000000 * (tmp109 + tmp110);
		    tmp120 = tmp109 - tmp110;
		    tmp113 = Y[-iostride];
		    tmp114 = Y[-3 * iostride];
		    tmp115 = K2_000000000 * (tmp113 - tmp114);
		    tmp121 = tmp114 + tmp113;
	       }
	  }
	  tmp108 = tmp105 + tmp107;
	  X[4 * iostride] = tmp108 - tmp111;
	  X[0] = tmp108 + tmp111;
	  tmp112 = tmp105 - tmp107;
	  X[6 * iostride] = tmp112 + tmp115;
	  X[2 * iostride] = tmp112 - tmp115;
	  {
	       fftw_real tmp119;
	       fftw_real tmp122;
	       fftw_real tmp123;
	       fftw_real tmp124;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp119 = tmp116 - tmp118;
	       tmp122 = K1_414213562 * (tmp120 - tmp121);
	       X[5 * iostride] = tmp119 - tmp122;
	       X[iostride] = tmp119 + tmp122;
	       tmp123 = tmp116 + tmp118;
	       tmp124 = K1_414213562 * (tmp120 + tmp121);
	       X[3 * iostride] = tmp123 - tmp124;
	       X[7 * iostride] = tmp123 + tmp124;
	  }
     }
     X = X + dist;
     Y = Y - dist;
     for (i = 2; i < m; i = i + 2, X = X + dist, Y = Y - dist, W = W + 7) {
	  fftw_real tmp29;
	  fftw_real tmp60;
	  fftw_real tmp46;
	  fftw_real tmp56;
	  fftw_real tmp70;
	  fftw_real tmp96;
	  fftw_real tmp82;
	  fftw_real tmp92;
	  fftw_real tmp36;
	  fftw_real tmp57;
	  fftw_real tmp53;
	  fftw_real tmp61;
	  fftw_real tmp73;
	  fftw_real tmp83;
	  fftw_real tmp76;
	  fftw_real tmp84;
	  ASSERT_ALIGNED_DOUBLE();
	  {
	       fftw_real tmp25;
	       fftw_real tmp68;
	       fftw_real tmp42;
	       fftw_real tmp81;
	       fftw_real tmp28;
	       fftw_real tmp80;
	       fftw_real tmp45;
	       fftw_real tmp69;
	       ASSERT_ALIGNED_DOUBLE();
	       {
		    fftw_real tmp23;
		    fftw_real tmp24;
		    fftw_real tmp40;
		    fftw_real tmp41;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp23 = X[0];
		    tmp24 = Y[-4 * iostride];
		    tmp25 = tmp23 + tmp24;
		    tmp68 = tmp23 - tmp24;
		    tmp40 = Y[0];
		    tmp41 = X[4 * iostride];
		    tmp42 = tmp40 - tmp41;
		    tmp81 = tmp40 + tmp41;
	       }
	       {
		    fftw_real tmp26;
		    fftw_real tmp27;
		    fftw_real tmp43;
		    fftw_real tmp44;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp26 = X[2 * iostride];
		    tmp27 = Y[-6 * iostride];
		    tmp28 = tmp26 + tmp27;
		    tmp80 = tmp26 - tmp27;
		    tmp43 = Y[-2 * iostride];
		    tmp44 = X[6 * iostride];
		    tmp45 = tmp43 - tmp44;
		    tmp69 = tmp43 + tmp44;
	       }
	       tmp29 = tmp25 + tmp28;
	       tmp60 = tmp25 - tmp28;
	       tmp46 = tmp42 + tmp45;
	       tmp56 = tmp42 - tmp45;
	       tmp70 = tmp68 - tmp69;
	       tmp96 = tmp68 + tmp69;
	       tmp82 = tmp80 + tmp81;
	       tmp92 = tmp81 - tmp80;
	  }
	  {
	       fftw_real tmp32;
	       fftw_real tmp71;
	       fftw_real tmp49;
	       fftw_real tmp72;
	       fftw_real tmp35;
	       fftw_real tmp74;
	       fftw_real tmp52;
	       fftw_real tmp75;
	       ASSERT_ALIGNED_DOUBLE();
	       {
		    fftw_real tmp30;
		    fftw_real tmp31;
		    fftw_real tmp47;
		    fftw_real tmp48;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp30 = X[iostride];
		    tmp31 = Y[-5 * iostride];
		    tmp32 = tmp30 + tmp31;
		    tmp71 = tmp30 - tmp31;
		    tmp47 = Y[-iostride];
		    tmp48 = X[5 * iostride];
		    tmp49 = tmp47 - tmp48;
		    tmp72 = tmp47 + tmp48;
	       }
	       {
		    fftw_real tmp33;
		    fftw_real tmp34;
		    fftw_real tmp50;
		    fftw_real tmp51;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp33 = Y[-7 * iostride];
		    tmp34 = X[3 * iostride];
		    tmp35 = tmp33 + tmp34;
		    tmp74 = tmp33 - tmp34;
		    tmp50 = Y[-3 * iostride];
		    tmp51 = X[7 * iostride];
		    tmp52 = tmp50 - tmp51;
		    tmp75 = tmp50 + tmp51;
	       }
	       tmp36 = tmp32 + tmp35;
	       tmp57 = tmp32 - tmp35;
	       tmp53 = tmp49 + tmp52;
	       tmp61 = tmp52 - tmp49;
	       tmp73 = tmp71 - tmp72;
	       tmp83 = tmp71 + tmp72;
	       tmp76 = tmp74 - tmp75;
	       tmp84 = tmp74 + tmp75;
	  }
	  X[0] = tmp29 + tmp36;
	  Y[-7 * iostride] = tmp46 + tmp53;
	  {
	       fftw_real tmp38;
	       fftw_real tmp54;
	       fftw_real tmp37;
	       fftw_real tmp39;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp38 = tmp29 - tmp36;
	       tmp54 = tmp46 - tmp53;
	       tmp37 = c_re(W[3]);
	       tmp39 = c_im(W[3]);
	       X[4 * iostride] = (tmp37 * tmp38) + (tmp39 * tmp54);
	       Y[-3 * iostride] = (tmp37 * tmp54) - (tmp39 * tmp38);
	  }
	  {
	       fftw_real tmp64;
	       fftw_real tmp66;
	       fftw_real tmp63;
	       fftw_real tmp65;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp64 = tmp57 + tmp56;
	       tmp66 = tmp60 + tmp61;
	       tmp63 = c_re(W[1]);
	       tmp65 = c_im(W[1]);
	       Y[-5 * iostride] = (tmp63 * tmp64) - (tmp65 * tmp66);
	       X[2 * iostride] = (tmp65 * tmp64) + (tmp63 * tmp66);
	  }
	  {
	       fftw_real tmp58;
	       fftw_real tmp62;
	       fftw_real tmp55;
	       fftw_real tmp59;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp58 = tmp56 - tmp57;
	       tmp62 = tmp60 - tmp61;
	       tmp55 = c_re(W[5]);
	       tmp59 = c_im(W[5]);
	       Y[-iostride] = (tmp55 * tmp58) - (tmp59 * tmp62);
	       X[6 * iostride] = (tmp59 * tmp58) + (tmp55 * tmp62);
	  }
	  {
	       fftw_real tmp94;
	       fftw_real tmp100;
	       fftw_real tmp98;
	       fftw_real tmp102;
	       fftw_real tmp93;
	       fftw_real tmp97;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp93 = K707106781 * (tmp73 - tmp76);
	       tmp94 = tmp92 + tmp93;
	       tmp100 = tmp92 - tmp93;
	       tmp97 = K707106781 * (tmp83 + tmp84);
	       tmp98 = tmp96 - tmp97;
	       tmp102 = tmp96 + tmp97;
	       {
		    fftw_real tmp91;
		    fftw_real tmp95;
		    fftw_real tmp99;
		    fftw_real tmp101;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp91 = c_re(W[2]);
		    tmp95 = c_im(W[2]);
		    Y[-4 * iostride] = (tmp91 * tmp94) - (tmp95 * tmp98);
		    X[3 * iostride] = (tmp95 * tmp94) + (tmp91 * tmp98);
		    tmp99 = c_re(W[6]);
		    tmp101 = c_im(W[6]);
		    Y[0] = (tmp99 * tmp100) - (tmp101 * tmp102);
		    X[7 * iostride] = (tmp101 * tmp100) + (tmp99 * tmp102);
	       }
	  }
	  {
	       fftw_real tmp78;
	       fftw_real tmp88;
	       fftw_real tmp86;
	       fftw_real tmp90;
	       fftw_real tmp77;
	       fftw_real tmp85;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp77 = K707106781 * (tmp73 + tmp76);
	       tmp78 = tmp70 - tmp77;
	       tmp88 = tmp70 + tmp77;
	       tmp85 = K707106781 * (tmp83 - tmp84);
	       tmp86 = tmp82 - tmp85;
	       tmp90 = tmp82 + tmp85;
	       {
		    fftw_real tmp67;
		    fftw_real tmp79;
		    fftw_real tmp87;
		    fftw_real tmp89;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp67 = c_re(W[4]);
		    tmp79 = c_im(W[4]);
		    X[5 * iostride] = (tmp67 * tmp78) + (tmp79 * tmp86);
		    Y[-2 * iostride] = (tmp67 * tmp86) - (tmp79 * tmp78);
		    tmp87 = c_re(W[0]);
		    tmp89 = c_im(W[0]);
		    X[iostride] = (tmp87 * tmp88) + (tmp89 * tmp90);
		    Y[-6 * iostride] = (tmp87 * tmp90) - (tmp89 * tmp88);
	       }
	  }
     }
     if (i == m) {
	  fftw_real tmp3;
	  fftw_real tmp7;
	  fftw_real tmp15;
	  fftw_real tmp20;
	  fftw_real tmp6;
	  fftw_real tmp12;
	  fftw_real tmp10;
	  fftw_real tmp21;
	  fftw_real tmp19;
	  fftw_real tmp22;
	  ASSERT_ALIGNED_DOUBLE();
	  {
	       fftw_real tmp1;
	       fftw_real tmp2;
	       fftw_real tmp13;
	       fftw_real tmp14;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp1 = X[0];
	       tmp2 = X[3 * iostride];
	       tmp3 = tmp1 + tmp2;
	       tmp7 = tmp1 - tmp2;
	       tmp13 = Y[0];
	       tmp14 = Y[-3 * iostride];
	       tmp15 = tmp13 + tmp14;
	       tmp20 = tmp13 - tmp14;
	  }
	  {
	       fftw_real tmp4;
	       fftw_real tmp5;
	       fftw_real tmp8;
	       fftw_real tmp9;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp4 = X[2 * iostride];
	       tmp5 = X[iostride];
	       tmp6 = tmp4 + tmp5;
	       tmp12 = tmp4 - tmp5;
	       tmp8 = Y[-2 * iostride];
	       tmp9 = Y[-iostride];
	       tmp10 = tmp8 + tmp9;
	       tmp21 = tmp8 - tmp9;
	  }
	  X[0] = K2_000000000 * (tmp3 + tmp6);
	  tmp19 = tmp3 - tmp6;
	  tmp22 = tmp20 - tmp21;
	  X[2 * iostride] = K1_414213562 * (tmp19 - tmp22);
	  X[6 * iostride] = -(K1_414213562 * (tmp19 + tmp22));
	  X[4 * iostride] = -(K2_000000000 * (tmp21 + tmp20));
	  {
	       fftw_real tmp11;
	       fftw_real tmp16;
	       fftw_real tmp17;
	       fftw_real tmp18;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp11 = tmp7 - tmp10;
	       tmp16 = tmp12 + tmp15;
	       X[iostride] = (K1_847759065 * tmp11) - (K765366864 * tmp16);
	       X[5 * iostride] = -((K765366864 * tmp11) + (K1_847759065 * tmp16));
	       tmp17 = tmp7 + tmp10;
	       tmp18 = tmp15 - tmp12;
	       X[3 * iostride] = (K765366864 * tmp17) - (K1_847759065 * tmp18);
	       X[7 * iostride] = -((K1_847759065 * tmp17) + (K765366864 * tmp18));
	  }
     }
}

static const int twiddle_order[] =
{1, 2, 3, 4, 5, 6, 7};
fftw_codelet_desc fftw_hc2hc_backward_8_desc =
{
     "fftw_hc2hc_backward_8",
     (void (*)()) fftw_hc2hc_backward_8,
     8,
     FFTW_BACKWARD,
     FFTW_HC2HC,
     190,
     7,
     twiddle_order,
};
