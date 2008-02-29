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
/* Generated on Tue May 18 13:56:02 EDT 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -hc2hc-backward 7 */

/*
 * This function contains 120 FP additions, 98 FP multiplications,
 * (or, 106 additions, 84 multiplications, 14 fused multiply/add),
 * 32 stack variables, and 56 memory accesses
 */
static const fftw_real K222520933 = FFTW_KONST(+0.222520933956314404288902564496794759466355569);
static const fftw_real K900968867 = FFTW_KONST(+0.900968867902419126236102319507445051165919162);
static const fftw_real K623489801 = FFTW_KONST(+0.623489801858733530525004884004239810632274731);
static const fftw_real K781831482 = FFTW_KONST(+0.781831482468029808708444526674057750232334519);
static const fftw_real K974927912 = FFTW_KONST(+0.974927912181823607018131682993931217232785801);
static const fftw_real K433883739 = FFTW_KONST(+0.433883739117558120475768332848358754609990728);
static const fftw_real K2_000000000 = FFTW_KONST(+2.000000000000000000000000000000000000000000000);
static const fftw_real K1_801937735 = FFTW_KONST(+1.801937735804838252472204639014890102331838324);
static const fftw_real K445041867 = FFTW_KONST(+0.445041867912628808577805128993589518932711138);
static const fftw_real K1_246979603 = FFTW_KONST(+1.246979603717467061050009768008479621264549462);
static const fftw_real K867767478 = FFTW_KONST(+0.867767478235116240951536665696717509219981456);
static const fftw_real K1_949855824 = FFTW_KONST(+1.949855824363647214036263365987862434465571601);
static const fftw_real K1_563662964 = FFTW_KONST(+1.563662964936059617416889053348115500464669037);

void fftw_hc2hc_backward_7(fftw_real *A, const fftw_complex *W, int iostride, int m, int dist)
{
     int i;
     fftw_real *X;
     fftw_real *Y;
     X = A;
     Y = A + (7 * iostride);
     {
	  fftw_real tmp84;
	  fftw_real tmp88;
	  fftw_real tmp86;
	  fftw_real tmp76;
	  fftw_real tmp79;
	  fftw_real tmp77;
	  fftw_real tmp78;
	  fftw_real tmp80;
	  fftw_real tmp87;
	  fftw_real tmp85;
	  fftw_real tmp81;
	  fftw_real tmp83;
	  fftw_real tmp82;
	  ASSERT_ALIGNED_DOUBLE();
	  tmp81 = Y[-2 * iostride];
	  tmp83 = Y[-iostride];
	  tmp82 = Y[-3 * iostride];
	  tmp84 = (K1_563662964 * tmp81) - (K1_949855824 * tmp82) - (K867767478 * tmp83);
	  tmp88 = (K867767478 * tmp81) + (K1_563662964 * tmp82) - (K1_949855824 * tmp83);
	  tmp86 = (K1_563662964 * tmp83) + (K1_949855824 * tmp81) + (K867767478 * tmp82);
	  tmp76 = X[0];
	  tmp79 = X[3 * iostride];
	  tmp77 = X[iostride];
	  tmp78 = X[2 * iostride];
	  tmp80 = tmp76 + (K1_246979603 * tmp78) - (K445041867 * tmp79) - (K1_801937735 * tmp77);
	  tmp87 = tmp76 + (K1_246979603 * tmp79) - (K1_801937735 * tmp78) - (K445041867 * tmp77);
	  tmp85 = tmp76 + (K1_246979603 * tmp77) - (K1_801937735 * tmp79) - (K445041867 * tmp78);
	  X[4 * iostride] = tmp80 - tmp84;
	  X[3 * iostride] = tmp80 + tmp84;
	  X[0] = tmp76 + (K2_000000000 * (tmp77 + tmp78 + tmp79));
	  X[2 * iostride] = tmp87 + tmp88;
	  X[5 * iostride] = tmp87 - tmp88;
	  X[iostride] = tmp85 - tmp86;
	  X[6 * iostride] = tmp85 + tmp86;
     }
     X = X + dist;
     Y = Y - dist;
     for (i = 2; i < m; i = i + 2, X = X + dist, Y = Y - dist, W = W + 6) {
	  fftw_real tmp14;
	  fftw_real tmp23;
	  fftw_real tmp17;
	  fftw_real tmp20;
	  fftw_real tmp39;
	  fftw_real tmp53;
	  fftw_real tmp66;
	  fftw_real tmp69;
	  fftw_real tmp57;
	  fftw_real tmp42;
	  fftw_real tmp24;
	  fftw_real tmp33;
	  fftw_real tmp27;
	  fftw_real tmp30;
	  fftw_real tmp46;
	  fftw_real tmp58;
	  fftw_real tmp70;
	  fftw_real tmp65;
	  fftw_real tmp54;
	  fftw_real tmp35;
	  ASSERT_ALIGNED_DOUBLE();
	  {
	       fftw_real tmp37;
	       fftw_real tmp36;
	       fftw_real tmp38;
	       fftw_real tmp21;
	       fftw_real tmp22;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp14 = X[0];
	       tmp21 = X[3 * iostride];
	       tmp22 = Y[-4 * iostride];
	       tmp23 = tmp21 + tmp22;
	       tmp37 = tmp21 - tmp22;
	       {
		    fftw_real tmp15;
		    fftw_real tmp16;
		    fftw_real tmp18;
		    fftw_real tmp19;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp15 = X[iostride];
		    tmp16 = Y[-6 * iostride];
		    tmp17 = tmp15 + tmp16;
		    tmp36 = tmp15 - tmp16;
		    tmp18 = X[2 * iostride];
		    tmp19 = Y[-5 * iostride];
		    tmp20 = tmp18 + tmp19;
		    tmp38 = tmp18 - tmp19;
	       }
	       tmp39 = (K433883739 * tmp36) + (K974927912 * tmp37) - (K781831482 * tmp38);
	       tmp53 = (K781831482 * tmp36) + (K974927912 * tmp38) + (K433883739 * tmp37);
	       tmp66 = (K974927912 * tmp36) - (K781831482 * tmp37) - (K433883739 * tmp38);
	       tmp69 = tmp14 + (K623489801 * tmp23) - (K900968867 * tmp20) - (K222520933 * tmp17);
	       tmp57 = tmp14 + (K623489801 * tmp17) - (K900968867 * tmp23) - (K222520933 * tmp20);
	       tmp42 = tmp14 + (K623489801 * tmp20) - (K222520933 * tmp23) - (K900968867 * tmp17);
	  }
	  {
	       fftw_real tmp44;
	       fftw_real tmp45;
	       fftw_real tmp43;
	       fftw_real tmp31;
	       fftw_real tmp32;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp24 = Y[0];
	       tmp31 = Y[-3 * iostride];
	       tmp32 = X[4 * iostride];
	       tmp33 = tmp31 - tmp32;
	       tmp44 = tmp31 + tmp32;
	       {
		    fftw_real tmp25;
		    fftw_real tmp26;
		    fftw_real tmp28;
		    fftw_real tmp29;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp25 = Y[-iostride];
		    tmp26 = X[6 * iostride];
		    tmp27 = tmp25 - tmp26;
		    tmp45 = tmp25 + tmp26;
		    tmp28 = Y[-2 * iostride];
		    tmp29 = X[5 * iostride];
		    tmp30 = tmp28 - tmp29;
		    tmp43 = tmp28 + tmp29;
	       }
	       tmp46 = (K781831482 * tmp43) - (K974927912 * tmp44) - (K433883739 * tmp45);
	       tmp58 = (K781831482 * tmp45) + (K974927912 * tmp43) + (K433883739 * tmp44);
	       tmp70 = (K433883739 * tmp43) + (K781831482 * tmp44) - (K974927912 * tmp45);
	       tmp65 = tmp24 + (K623489801 * tmp33) - (K900968867 * tmp30) - (K222520933 * tmp27);
	       tmp54 = tmp24 + (K623489801 * tmp27) - (K900968867 * tmp33) - (K222520933 * tmp30);
	       tmp35 = tmp24 + (K623489801 * tmp30) - (K222520933 * tmp33) - (K900968867 * tmp27);
	  }
	  X[0] = tmp14 + tmp17 + tmp20 + tmp23;
	  {
	       fftw_real tmp61;
	       fftw_real tmp63;
	       fftw_real tmp60;
	       fftw_real tmp62;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp61 = tmp54 - tmp53;
	       tmp63 = tmp57 + tmp58;
	       tmp60 = c_re(W[5]);
	       tmp62 = c_im(W[5]);
	       Y[0] = (tmp60 * tmp61) - (tmp62 * tmp63);
	       X[6 * iostride] = (tmp62 * tmp61) + (tmp60 * tmp63);
	  }
	  {
	       fftw_real tmp73;
	       fftw_real tmp75;
	       fftw_real tmp72;
	       fftw_real tmp74;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp73 = tmp66 + tmp65;
	       tmp75 = tmp69 + tmp70;
	       tmp72 = c_re(W[1]);
	       tmp74 = c_im(W[1]);
	       Y[-4 * iostride] = (tmp72 * tmp73) - (tmp74 * tmp75);
	       X[2 * iostride] = (tmp74 * tmp73) + (tmp72 * tmp75);
	  }
	  {
	       fftw_real tmp67;
	       fftw_real tmp71;
	       fftw_real tmp64;
	       fftw_real tmp68;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp67 = tmp65 - tmp66;
	       tmp71 = tmp69 - tmp70;
	       tmp64 = c_re(W[4]);
	       tmp68 = c_im(W[4]);
	       Y[-iostride] = (tmp64 * tmp67) - (tmp68 * tmp71);
	       X[5 * iostride] = (tmp68 * tmp67) + (tmp64 * tmp71);
	  }
	  Y[-6 * iostride] = tmp24 + tmp27 + tmp30 + tmp33;
	  {
	       fftw_real tmp40;
	       fftw_real tmp47;
	       fftw_real tmp34;
	       fftw_real tmp41;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp40 = tmp35 - tmp39;
	       tmp47 = tmp42 - tmp46;
	       tmp34 = c_re(W[3]);
	       tmp41 = c_im(W[3]);
	       Y[-2 * iostride] = (tmp34 * tmp40) - (tmp41 * tmp47);
	       X[4 * iostride] = (tmp41 * tmp40) + (tmp34 * tmp47);
	  }
	  {
	       fftw_real tmp49;
	       fftw_real tmp51;
	       fftw_real tmp48;
	       fftw_real tmp50;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp49 = tmp39 + tmp35;
	       tmp51 = tmp42 + tmp46;
	       tmp48 = c_re(W[2]);
	       tmp50 = c_im(W[2]);
	       Y[-3 * iostride] = (tmp48 * tmp49) - (tmp50 * tmp51);
	       X[3 * iostride] = (tmp50 * tmp49) + (tmp48 * tmp51);
	  }
	  {
	       fftw_real tmp55;
	       fftw_real tmp59;
	       fftw_real tmp52;
	       fftw_real tmp56;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp55 = tmp53 + tmp54;
	       tmp59 = tmp57 - tmp58;
	       tmp52 = c_re(W[0]);
	       tmp56 = c_im(W[0]);
	       Y[-5 * iostride] = (tmp52 * tmp55) - (tmp56 * tmp59);
	       X[iostride] = (tmp56 * tmp55) + (tmp52 * tmp59);
	  }
     }
     if (i == m) {
	  fftw_real tmp9;
	  fftw_real tmp13;
	  fftw_real tmp11;
	  fftw_real tmp1;
	  fftw_real tmp4;
	  fftw_real tmp2;
	  fftw_real tmp3;
	  fftw_real tmp5;
	  fftw_real tmp12;
	  fftw_real tmp10;
	  fftw_real tmp6;
	  fftw_real tmp8;
	  fftw_real tmp7;
	  ASSERT_ALIGNED_DOUBLE();
	  tmp6 = Y[-2 * iostride];
	  tmp8 = Y[0];
	  tmp7 = Y[-iostride];
	  tmp9 = (K1_563662964 * tmp6) + (K1_949855824 * tmp7) + (K867767478 * tmp8);
	  tmp13 = (K1_563662964 * tmp7) - (K1_949855824 * tmp8) - (K867767478 * tmp6);
	  tmp11 = (K1_949855824 * tmp6) - (K1_563662964 * tmp8) - (K867767478 * tmp7);
	  tmp1 = X[3 * iostride];
	  tmp4 = X[0];
	  tmp2 = X[2 * iostride];
	  tmp3 = X[iostride];
	  tmp5 = (K445041867 * tmp3) + (K1_801937735 * tmp4) - (K1_246979603 * tmp2) - tmp1;
	  tmp12 = (K1_801937735 * tmp2) + (K445041867 * tmp4) - (K1_246979603 * tmp3) - tmp1;
	  tmp10 = tmp1 + (K1_246979603 * tmp4) - (K1_801937735 * tmp3) - (K445041867 * tmp2);
	  X[iostride] = tmp5 - tmp9;
	  X[6 * iostride] = -(tmp5 + tmp9);
	  X[0] = tmp1 + (K2_000000000 * (tmp2 + tmp3 + tmp4));
	  X[4 * iostride] = tmp13 - tmp12;
	  X[3 * iostride] = tmp12 + tmp13;
	  X[5 * iostride] = tmp11 - tmp10;
	  X[2 * iostride] = tmp10 + tmp11;
     }
}

static const int twiddle_order[] =
{1, 2, 3, 4, 5, 6};
fftw_codelet_desc fftw_hc2hc_backward_7_desc =
{
     "fftw_hc2hc_backward_7",
     (void (*)()) fftw_hc2hc_backward_7,
     7,
     FFTW_BACKWARD,
     FFTW_HC2HC,
     168,
     6,
     twiddle_order,
};
