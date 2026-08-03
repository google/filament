/*
 * tocio - AOT C-source codegen. Emits a standalone .c implementing the op chain
 * as a straight-line per-pixel loop over interleaved RGBA float. LUT data is
 * embedded as static const arrays; constants are bit-exact hex floats. The
 * emitted file is dependency-free (its own libm-free pow/log2/exp2).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

static void emit_hf(toc_sb *sb, float v) { toc_sb_hexfloat(sb, v); }

/* Emit a 4x4 matrix*rgba+offset into fresh temps then write back. */
static void op_matrix(toc_sb *sb, const toc_op *op) {
    const float *m = op->u.matrix.m, *o = op->u.matrix.off;
    static const char *up[4] = {"R", "G", "B", "A"};
    int r;
    toc_sb_puts(sb, "  { float R,G,B,A;\n");
    for (r = 0; r < 4; ++r) {
        toc_sb_puts(sb, "  ");
        toc_sb_puts(sb, up[r]);
        toc_sb_puts(sb, " = ");
        emit_hf(sb, m[0 + r]); toc_sb_puts(sb, "*r + ");
        emit_hf(sb, m[4 + r]); toc_sb_puts(sb, "*g + ");
        emit_hf(sb, m[8 + r]); toc_sb_puts(sb, "*b + ");
        emit_hf(sb, m[12 + r]); toc_sb_puts(sb, "*a + ");
        emit_hf(sb, o[r]); toc_sb_puts(sb, ";\n");
    }
    toc_sb_puts(sb, "  r=R; g=G; b=B; a=A; }\n");
}

static void op_range(toc_sb *sb, const toc_op *op) {
    static const char *cn[4] = {"r", "g", "b", "a"};
    int c;
    for (c = 0; c < 4; ++c) {
        toc_sb_puts(sb, "  ");
        toc_sb_puts(sb, cn[c]);
        toc_sb_puts(sb, " = ");
        toc_sb_puts(sb, cn[c]);
        toc_sb_puts(sb, "*");
        emit_hf(sb, op->u.range.scale[c]);
        toc_sb_puts(sb, " + ");
        emit_hf(sb, op->u.range.offset[c]);
        toc_sb_puts(sb, ";");
        if (op->u.range.clamp_lo) {
            toc_sb_puts(sb, " if(");
            toc_sb_puts(sb, cn[c]);
            toc_sb_puts(sb, "<");
            emit_hf(sb, op->u.range.min[c]);
            toc_sb_puts(sb, ") ");
            toc_sb_puts(sb, cn[c]);
            toc_sb_puts(sb, "=");
            emit_hf(sb, op->u.range.min[c]);
            toc_sb_puts(sb, ";");
        }
        if (op->u.range.clamp_hi) {
            toc_sb_puts(sb, " if(");
            toc_sb_puts(sb, cn[c]);
            toc_sb_puts(sb, ">");
            emit_hf(sb, op->u.range.max[c]);
            toc_sb_puts(sb, ") ");
            toc_sb_puts(sb, cn[c]);
            toc_sb_puts(sb, "=");
            emit_hf(sb, op->u.range.max[c]);
            toc_sb_puts(sb, ";");
        }
        toc_sb_puts(sb, "\n");
    }
}

static void op_exponent(toc_sb *sb, const toc_op *op) {
    static const char *cn[4] = {"r", "g", "b", "a"};
    int c, mir = op->u.exponent.mirror;
    for (c = 0; c < 4; ++c) {
        const char *v = cn[c];
        toc_sb_puts(sb, "  "); toc_sb_puts(sb, v); toc_sb_puts(sb, " = ");
        if (mir) {
            /* sign(v)*tc_powf(|v|, e) */
            toc_sb_puts(sb, "("); toc_sb_puts(sb, v);
            toc_sb_puts(sb, "<0.0f?-1.0f:1.0f)*tc_powf(");
            toc_sb_puts(sb, v); toc_sb_puts(sb, "<0.0f?-");
            toc_sb_puts(sb, v); toc_sb_puts(sb, ":"); toc_sb_puts(sb, v);
            toc_sb_puts(sb, ", ");
        } else {
            toc_sb_puts(sb, "tc_powf("); toc_sb_puts(sb, v);
            toc_sb_puts(sb, ">0.0f?"); toc_sb_puts(sb, v);
            toc_sb_puts(sb, ":0.0f, ");
        }
        emit_hf(sb, op->u.exponent.e[c]);
        toc_sb_puts(sb, ");\n");
    }
}

static void op_exp_linear(toc_sb *sb, const toc_op *op) {
    static const char *cn[3] = {"r", "g", "b"};
    int c, mir = op->u.exp_linear.mirror;
    for (c = 0; c < 3; ++c) {
        float scale = op->u.exp_linear.scale[c], off = op->u.exp_linear.offset[c];
        float g = op->u.exp_linear.gamma[c], brk = op->u.exp_linear.breakpoint[c];
        float slope = op->u.exp_linear.slope[c];
        const char *v = cn[c];
        const char *iv = mir ? "_a" : v; /* input the curve reads */
        toc_sb_puts(sb, "  ");
        if (mir) {
            /* {float _s=sign(v),_a=|v|; v = _s*(curve(_a));} */
            toc_sb_puts(sb, "{float _s="); toc_sb_puts(sb, v);
            toc_sb_puts(sb, "<0.0f?-1.0f:1.0f,_a="); toc_sb_puts(sb, v);
            toc_sb_puts(sb, "<0.0f?-"); toc_sb_puts(sb, v); toc_sb_puts(sb, ":");
            toc_sb_puts(sb, v); toc_sb_puts(sb, ";"); toc_sb_puts(sb, v);
            toc_sb_puts(sb, " = _s*((");
        } else {
            toc_sb_puts(sb, v); toc_sb_puts(sb, " = (");
        }
        if (!op->u.exp_linear.inverse) {
            toc_sb_puts(sb, iv); toc_sb_puts(sb, ">");
            emit_hf(sb, brk); toc_sb_puts(sb, ") ? tc_powf(");
            toc_sb_puts(sb, iv); toc_sb_puts(sb, "*"); emit_hf(sb, scale);
            toc_sb_puts(sb, "+"); emit_hf(sb, off); toc_sb_puts(sb, ", ");
            emit_hf(sb, g); toc_sb_puts(sb, ") : ");
            toc_sb_puts(sb, iv); toc_sb_puts(sb, "*"); emit_hf(sb, slope);
        } else {
            toc_sb_puts(sb, iv); toc_sb_puts(sb, ">");
            emit_hf(sb, brk * slope); toc_sb_puts(sb, ") ? (tc_powf(");
            toc_sb_puts(sb, iv); toc_sb_puts(sb, ", "); emit_hf(sb, 1.0f / g);
            toc_sb_puts(sb, ")-"); emit_hf(sb, off); toc_sb_puts(sb, ")/");
            emit_hf(sb, scale); toc_sb_puts(sb, " : ");
            toc_sb_puts(sb, iv); toc_sb_puts(sb, "/"); emit_hf(sb, slope);
        }
        toc_sb_puts(sb, mir ? ");}\n" : ";\n");
    }
}

static void op_log(toc_sb *sb, const toc_op *op) {
    static const char *cn[3] = {"r", "g", "b"};
    int c;
    for (c = 0; c < 3; ++c) {
        toc_sb_puts(sb, "  ");
        toc_sb_puts(sb, cn[c]);
        toc_sb_puts(sb, " = ");
        if (!op->u.log.inverse) {
            toc_sb_puts(sb, "("); emit_hf(sb, op->u.log.log_slope[c]);
            toc_sb_puts(sb, "*tc_log2f((");
            emit_hf(sb, op->u.log.lin_slope[c]); toc_sb_puts(sb, "*");
            toc_sb_puts(sb, cn[c]); toc_sb_puts(sb, "+");
            emit_hf(sb, op->u.log.lin_offset[c]); toc_sb_puts(sb, "))/tc_log2f(");
            emit_hf(sb, op->u.log.base); toc_sb_puts(sb, ")+");
            emit_hf(sb, op->u.log.log_offset[c]); toc_sb_puts(sb, ")");
        } else {
            toc_sb_puts(sb, "(tc_raisef(");
            emit_hf(sb, op->u.log.base); toc_sb_puts(sb, ", (");
            toc_sb_puts(sb, cn[c]); toc_sb_puts(sb, "-");
            emit_hf(sb, op->u.log.log_offset[c]); toc_sb_puts(sb, ")/");
            emit_hf(sb, op->u.log.log_slope[c]); toc_sb_puts(sb, ")-");
            emit_hf(sb, op->u.log.lin_offset[c]); toc_sb_puts(sb, ")/");
            emit_hf(sb, op->u.log.lin_slope[c]);
        }
        toc_sb_puts(sb, ";\n");
    }
}

static void op_cdl(toc_sb *sb, const toc_op *op) {
    const float *L = op->u.cdl.luma;
    float lr = L[0], lg = L[1], lb = L[2];
    int c;
    static const char *cn[3] = {"r", "g", "b"};
    if (lr == 0 && lg == 0 && lb == 0) { lr = 0.2126f; lg = 0.7152f; lb = 0.0722f; }
    if (op->u.cdl.inverse) {
        float sat = op->u.cdl.saturation;
        toc_sb_puts(sb, "  { float v0,v1,v2,L;\n  L = ");
        emit_hf(sb, lr); toc_sb_puts(sb, "*r+"); emit_hf(sb, lg);
        toc_sb_puts(sb, "*g+"); emit_hf(sb, lb); toc_sb_puts(sb, "*b;\n");
        for (c = 0; c < 3; ++c) {
            float slope = op->u.cdl.slope[c];
            float ipow = op->u.cdl.power[c] != 0.0f ? 1.0f / op->u.cdl.power[c]
                                                    : 1.0f;
            toc_sb_puts(sb, "  v"); toc_sb_int(sb, c); toc_sb_puts(sb, " = L");
            if (sat != 0.0f) { /* undo saturation (luma preserved) */
                toc_sb_puts(sb, "+("); toc_sb_puts(sb, cn[c]);
                toc_sb_puts(sb, "-L)*"); emit_hf(sb, 1.0f / sat);
            }
            toc_sb_putc(sb, ';');
            if (op->u.cdl.clamp) {
                toc_sb_puts(sb, " if(v"); toc_sb_int(sb, c);
                toc_sb_puts(sb, "<0.0f)v"); toc_sb_int(sb, c);
                toc_sb_puts(sb, "=0.0f; if(v"); toc_sb_int(sb, c);
                toc_sb_puts(sb, ">1.0f)v"); toc_sb_int(sb, c); toc_sb_puts(sb, "=1.0f;");
            }
            toc_sb_puts(sb, " v"); toc_sb_int(sb, c); toc_sb_puts(sb, "=v");
            toc_sb_int(sb, c); toc_sb_puts(sb, ">=0.0f?tc_powf(v"); toc_sb_int(sb, c);
            toc_sb_puts(sb, ", "); emit_hf(sb, ipow);
            toc_sb_puts(sb, "):v"); toc_sb_int(sb, c); toc_sb_puts(sb, ";\n  ");
            toc_sb_puts(sb, cn[c]); toc_sb_puts(sb, " = (v"); toc_sb_int(sb, c);
            toc_sb_puts(sb, "+"); emit_hf(sb, -op->u.cdl.offset[c]); /* avoid -- */
            toc_sb_puts(sb, ")*"); emit_hf(sb, slope != 0.0f ? 1.0f / slope : 1.0f);
            toc_sb_puts(sb, ";\n");
        }
        toc_sb_puts(sb, "  }\n");
        return;
    }
    toc_sb_puts(sb, "  { float v0,v1,v2,L;\n");
    for (c = 0; c < 3; ++c) {
        toc_sb_puts(sb, "  v"); toc_sb_int(sb, c); toc_sb_puts(sb, " = ");
        toc_sb_puts(sb, cn[c]); toc_sb_puts(sb, "*"); emit_hf(sb, op->u.cdl.slope[c]);
        toc_sb_puts(sb, "+"); emit_hf(sb, op->u.cdl.offset[c]); toc_sb_puts(sb, ";");
        if (op->u.cdl.clamp) {
            toc_sb_puts(sb, " if(v"); toc_sb_int(sb, c);
            toc_sb_puts(sb, "<0.0f)v"); toc_sb_int(sb, c);
            toc_sb_puts(sb, "=0.0f; if(v"); toc_sb_int(sb, c);
            toc_sb_puts(sb, ">1.0f)v"); toc_sb_int(sb, c); toc_sb_puts(sb, "=1.0f;");
        }
        toc_sb_puts(sb, " v"); toc_sb_int(sb, c); toc_sb_puts(sb, "=v");
        toc_sb_int(sb, c); toc_sb_puts(sb, ">=0.0f?tc_powf(v"); toc_sb_int(sb, c);
        toc_sb_puts(sb, ", "); emit_hf(sb, op->u.cdl.power[c]);
        toc_sb_puts(sb, "):v"); toc_sb_int(sb, c); toc_sb_puts(sb, ";\n");
    }
    toc_sb_puts(sb, "  L = "); emit_hf(sb, lr); toc_sb_puts(sb, "*v0+");
    emit_hf(sb, lg); toc_sb_puts(sb, "*v1+"); emit_hf(sb, lb);
    toc_sb_puts(sb, "*v2;\n");
    for (c = 0; c < 3; ++c) {
        toc_sb_puts(sb, "  "); toc_sb_puts(sb, cn[c]);
        toc_sb_puts(sb, " = L+"); emit_hf(sb, op->u.cdl.saturation);
        toc_sb_puts(sb, "*(v"); toc_sb_int(sb, c); toc_sb_puts(sb, "-L);");
        if (op->u.cdl.clamp) {
            toc_sb_puts(sb, " if("); toc_sb_puts(sb, cn[c]);
            toc_sb_puts(sb, "<0.0f)"); toc_sb_puts(sb, cn[c]);
            toc_sb_puts(sb, "=0.0f; if("); toc_sb_puts(sb, cn[c]);
            toc_sb_puts(sb, ">1.0f)"); toc_sb_puts(sb, cn[c]); toc_sb_puts(sb, "=1.0f;");
        }
        toc_sb_puts(sb, "\n");
    }
    toc_sb_puts(sb, "  }\n");
}

/* LUT ops call emitted helpers with the embedded array name. */
static void emit_lut_array(toc_sb *sb, int idx, const float *data, size_t n) {
    size_t i;
    toc_sb_puts(sb, "static const float tc_lut");
    toc_sb_int(sb, idx);
    toc_sb_puts(sb, "[] = {");
    for (i = 0; i < n; ++i) {
        if (i) toc_sb_putc(sb, ',');
        emit_hf(sb, data[i]);
    }
    toc_sb_puts(sb, "};\n");
}

/* ---- helper source snippets --------------------------------------------- */
static const char *MATH_SRC =
    "static float tc_log2f(float x){union{float f;unsigned u;}v;int e;float "
    "m,t,t2,ln;if(!(x>0.0f))return x<0.0f?(0.0f/0.0f):-1e38f;v.f=x;e=(int)((v.u"
    ">>23)&0xffu)-127;v.u=(v.u&0x007fffffu)|0x3f800000u;m=v.f;t=(m-1.0f)/(m+1.0"
    "f);t2=t*t;ln=2.0f*t*(1.0f+t2*(1.0f/3.0f+t2*(1.0f/5.0f+t2*(1.0f/7.0f+t2*(1."
    "0f/9.0f)))));return (float)e+ln*1.4426950408889634f;}\n"
    "static float tc_exp2f(float x){union{unsigned u;float f;}v;float "
    "k,f,g,p;int ki;if(x>127.0f)return 1e38f;if(x<-126.0f)return 0.0f;k=(x>=0.0"
    "f)?(float)(int)(x+0.5f):(float)(int)(x-0.5f);f=x-k;g=f*0.6931471805599453f"
    ";p=1.0f+g*(1.0f+g*(0.5f+g*(1.0f/6.0f+g*(1.0f/24.0f+g*(1.0f/120.0f+g*(1.0f/"
    "720.0f))))));ki=(int)k;v.u=(unsigned)((ki+127)<<23);return p*v.f;}\n"
    "static float tc_powf(float x,float y){if(!(x>0.0f))return "
    "x==0.0f?(y>0.0f?0.0f:1.0f):(0.0f/0.0f);return tc_exp2f(y*tc_log2f(x));}\n"
    "static float tc_raisef(float b,float x){if(!(b>0.0f))return "
    "0.0f;return tc_exp2f(x*tc_log2f(b));}\n";

static const char *LUT1D_SRC =
    "static void tc_lut1d(const float*d,int N,int ch,float lo,float hi,float*c)"
    "{int k;float den=hi-lo;for(k=0;k<3;++k){float u=den!=0.0f?(c[k]-lo)/den:0."
    "0f,g,f,a,b;int i;if(!(u>0.0f))u=0.0f;if(u>1.0f)u=1.0f;g=u*(float)(N-1);i=("
    "int)g;if(i>=N-1)i=N-2;f=g-(float)i;if(ch==1){a=d[i];b=d[i+1];}else{a=d[i*3"
    "+k];b=d[(i+1)*3+k];}c[k]=a+(b-a)*f;}}\n";

/* FixedFunction helpers (toc_fixedfunc_apply_pixel inlined for freestanding) */
static const char *FIXEDFUNC_SRC_GLOW =
    "static float tc_glow_yc(float r,float g,float b){"
    "float c=b*(b-g)+g*(g-r)+r*(r-b);float ch=c>0?sqrtf(c):0;"
    "return(b+g+r+1.75f*ch)/3.0f;}\n"
    "static float tc_glow_sat(float r,float g,float b){"
    "float mn,rng;"
    "mn=r<g?r:g;mn=mn<b?mn:b;rng=(r>g?r:g)>b?(r>g?r:g):b;"
    "rng=rng<1e-10f?1e-10f:rng;"
    "return((rng<1e-10f?1e-10f:rng)-(mn<1e-10f?1e-10f:mn))/rng;}\n"
    "static float tc_glow_sig(float s){"
    "float x=(s-0.4f)*5.0f,sg,t,ss;"
    "sg=x>=0?1.0f:-1.0f;t=0.5f*sg*x;t=t>1.0f?1.0f:t;t=t<0.0f?0.0f:t;t=1.0f-t;"
    "ss=(1.0f+sg*(1.0f-t*t))*0.5f;return ss;}\n"
    "static void tc_glow(float*px,float gg,float gm,int inv){"
    "float r=px[0],g=px[1],b=px[2];"
    "float yc=tc_glow_yc(r,g,b);"
    "float sa=tc_glow_sat(r,g,b);"
    "float s=tc_glow_sig(sa);"
    "float gv=gg*s,go;"
    "if(yc>=gm*2.0f)go=0.0f;"
    "else if(!inv){"
    "go=yc<=gm*2.0f/3.0f?gv:gv*(gm/yc-0.5f);"
    "}else{"
    "go=yc<=(1.0f+gv)*gm*2.0f/3.0f?-gv/(1.0f+gv):gv*(gm/yc-0.5f)/(gv*0.5f-1.0f);"
    "}float f=1.0f+go;px[0]=r*f;px[1]=g*f;px[2]=b*f;}\n";

static const char *FIXEDFUNC_SRC_DARKTODIM =
    "static void tc_darktodim(float*px,float g){"
    "float y=0.27222872f*px[0]+0.67408177f*px[1]+0.05368952f*px[2];"
    "y=y<1e-10f?1e-10f:y;y=tc_powf(y,g-1.0f);"
    "px[0]*=y;px[1]*=y;px[2]*=y;}\n";

static const char *FIXEDFUNC_SRC_RGB2HSV =
    "static void tc_rgb2hsv(float*px){"
    "float r=px[0],g=px[1],b=px[2];"
    "float mn=r<g?r:g;mn=mn<b?mn:b;"
    "float mx=r>g?r:g;mx=mx>b?mx:b;"
    "float v=mx,s=0.0f,h=0.0f,d;"
    "if(mn!=mx){d=mx-mn;if(mx!=0)s=d/mx;"
    "if(r==mx)h=(g-b)/d;else if(g==mx)h=2.0f+(b-r)/d;else h=4.0f+(r-g)/d;"
    "if(h<0)h+=6.0f;h*=0.16666667f;}"
    "if(mn<0.0f)v+=mn;if(-mn>mx)s=(mx-mn)/-mn;"
    "px[0]=h;px[1]=s;px[2]=v;}\n"
    "static void tc_hsv2rgb(float*px){"
    "float h=(px[0]-(float)(int)px[0])*6.0f;"
    "float s=px[1];s=s<0?0:s;s=s>1.999f?1.999f:s;"
    "float v=px[2];"
    "float r=(h<3?((h-3)>-(h-3)?(h-3):-(h-3)):((h-3)>-(h-3)?(h-3):-(h-3)))-1.0f;"
    "r=r<0?0:r;r=r>1?1:r;"
    "float gv=2.0f-(h<2?(2-h):(h-2));gv=gv<0?0:gv;gv=gv>1?1:gv;"
    "float bv=2.0f-(h<4?(4-h):(h-4));bv=bv<0?0:bv;bv=bv>1?1:bv;"
    "float mx=v,mn=v*(1.0f-s),dd;"
    "if(s>1.0f){mn=v*(1.0f-s)/(2.0f-s);mx=v-mn;}"
    "if(v<0.0f){mn=v/(2.0f-s);mx=v-mn;}dd=mx-mn;"
    "px[0]=r*dd+mn;px[1]=gv*dd+mn;px[2]=bv*dd+mn;}\n";

static const char *FIXEDFUNC_SRC_XYZ =
    "static void tc_xyz2xyy(float*px){"
    "float d=px[0]+px[1]+px[2];d=d==0?0:1.0f/d;"
    "px[0]=px[0]*d;px[1]=px[1]*d;}\n"
    "static void tc_xyy2xyz(float*px){"
    "float d=px[1]==0?0:1.0f/px[1];"
    "px[0]=px[2]*px[0]*d;px[2]=px[2]*(1.0f-px[0]-px[1])*d;}\n"
    "static void tc_xyz2uvy(float*px){"
    "float d=px[0]+15.0f*px[1]+3.0f*px[2];d=d==0?0:1.0f/d;"
    "px[0]=4.0f*px[0]*d;px[1]=9.0f*px[1]*d;}\n"
    "static void tc_uvy2xyz(float*px){"
    "float d=px[1]==0?0:1.0f/px[1];"
    "px[0]=2.25f*px[2]*px[0]*d;"
    "px[2]=0.75f*px[2]*(4.0f-px[0]-6.6666667f*px[1])*d;}\n"
    "static void tc_xyz2luv(float*px){"
    "float X=px[0],Y=px[1],Z=px[2];"
    "float d=X+15.0f*Y+3.0f*Z;d=d==0?0:1.0f/d;"
    "float u=4.0f*X*d,v=9.0f*Y*d;"
    "float ls=Y<=0.008856452f?9.032963f*Y:1.16f*tc_powf(Y,0.33333333f)-0.16f;"
    "px[0]=ls;px[1]=13.0f*ls*(u-0.19783001f);px[2]=13.0f*ls*(v-0.46831999f);}\n"
    "static void tc_luv2xyz(float*px){"
    "float ls=px[0],us=px[1],vs=px[2];"
    "float d=ls==0?0:0.07692308f/ls;"
    "float u=us*d+0.19783001f,v=vs*d+0.46831999f;"
    "float t=(ls+0.16f)*0.86206897f;"
    "float Y=ls<=0.08f?0.11070565f*ls:t*t*t;"
    "float dd=v==0?0:0.25f/v;"
    "px[0]=9.0f*Y*u*dd;px[1]=Y;px[2]=Y*(12.0f-3.0f*u-20.0f*v)*dd;}\n";

/* PQ (ST 2084) + HLG transfer functions (libm-free; same as the interpreter). */
static const char *FIXEDFUNC_SRC_HDR =
    "static float tc_pq_enc(float L){float m1=0.1593017578125f,m2=78.84375f,"
    "c1=0.8359375f,c2=18.8515625f,c3=18.6875f,Lm;if(L<=0.0f)return 0.0f;"
    "Lm=tc_powf(L*0.01f,m1);return tc_powf((c1+c2*Lm)/(1.0f+c3*Lm),m2);}\n"
    "static float tc_pq_dec(float N){float m1=0.1593017578125f,m2=78.84375f,"
    "c1=0.8359375f,c2=18.8515625f,c3=18.6875f,Np,nu,de;if(N<=0.0f)return 0.0f;"
    "Np=tc_powf(N,1.0f/m2);nu=Np-c1;if(nu<0.0f)nu=0.0f;de=c2-c3*Np;"
    "if(de<=0.0f)return 0.0f;return tc_powf(nu/de,1.0f/m1)*100.0f;}\n"
    "static float tc_hlg_enc(float E){float a=0.17883277f,b=0.28466892f,"
    "c=0.55991073f;if(E<=0.0f)return 0.0f;if(E<=1.0f/12.0f)return "
    "tc_exp2f(0.5f*tc_log2f(3.0f*E));return a*(tc_log2f(12.0f*E-b)*"
    "0.6931471805599453f)+c;}\n"
    "static float tc_hlg_dec(float E){float a=0.17883277f,b=0.28466892f,"
    "c=0.55991073f;if(E<=0.0f)return 0.0f;if(E<=0.5f)return E*E/3.0f;"
    "return (tc_exp2f(((E-c)/a)*1.4426950408889634f)+b)/12.0f;}\n";

static const char *LUT3D_SRC =
    "static void tc_lut3d(const float*d,int N,const float*dn,const float*dx,int"
    " tet,float*c){int ic[3],k;float f[3];for(k=0;k<3;++k){float den=dx[k]-dn[k"
    "],u=den!=0.0f?(c[k]-dn[k])/den:0.0f,g;if(!(u>0.0f))u=0.0f;if(u>1.0f)u=1.0f"
    ";g=u*(float)(N-1);ic[k]=(int)g;if(ic[k]>=N-1)ic[k]=N-2;f[k]=g-(float)ic[k]"
    ";}\n"
    "#define C(R,G,B) (d+((((size_t)(B)*N+(G))*N+(R))*3))\n"
    "{const float*c000=C(ic[0],ic[1],ic[2]);const float*c100=C(ic[0]+1,ic[1],ic"
    "[2]);const float*c010=C(ic[0],ic[1]+1,ic[2]);const float*c110=C(ic[0]+1,ic"
    "[1]+1,ic[2]);const float*c001=C(ic[0],ic[1],ic[2]+1);const float*c101=C(ic"
    "[0]+1,ic[1],ic[2]+1);const float*c011=C(ic[0],ic[1]+1,ic[2]+1);const float"
    "*c111=C(ic[0]+1,ic[1]+1,ic[2]+1);float fr=f[0],fg=f[1],fb=f[2];for(k=0;k<3"
    ";++k){float v;if(tet){if(fr>=fg&&fg>=fb)v=c000[k]+fr*(c100[k]-c000[k])+fg*"
    "(c110[k]-c100[k])+fb*(c111[k]-c110[k]);else if(fr>=fb&&fb>=fg)v=c000[k]+fr"
    "*(c100[k]-c000[k])+fb*(c101[k]-c100[k])+fg*(c111[k]-c101[k]);else if(fb>=f"
    "r&&fr>=fg)v=c000[k]+fb*(c001[k]-c000[k])+fr*(c101[k]-c001[k])+fg*(c111[k]-"
    "c101[k]);else if(fg>=fr&&fr>=fb)v=c000[k]+fg*(c010[k]-c000[k])+fr*(c110[k]"
    "-c010[k])+fb*(c111[k]-c110[k]);else if(fg>=fb&&fb>=fr)v=c000[k]+fg*(c010[k"
    "]-c000[k])+fb*(c011[k]-c010[k])+fr*(c111[k]-c011[k]);else v=c000[k]+fb*(c0"
    "01[k]-c000[k])+fg*(c011[k]-c001[k])+fr*(c111[k]-c011[k]);}else{float "
    "x00=c000[k]+(c100[k]-c000[k])*fr,x01=c001[k]+(c101[k]-c001[k])*fr,x10=c010"
    "[k]+(c110[k]-c010[k])*fr,x11=c011[k]+(c111[k]-c011[k])*fr,y0=x00+(x10-x00)"
    "*fg,y1=x01+(x11-x01)*fg;v=y0+(y1-y0)*fb;}c[k]=v;}}\n#undef C\n}\n";

/* ACES Red Modifier + Gamut Compression. Uses the bundled tc_powf/tc_log2f/
 * tc_exp2f so the generated code reproduces the interpreter's approximations
 * exactly (tc_sqrtf == toc_sqrtf, tc_atan2f == ff_atan2f). Needs MATH_SRC. */
static const char *FIXEDFUNC_SRC_ACES =
    "static float tc_sqrtf(float x){return x>0.0f?tc_exp2f(0.5f*tc_log2f(x)):0.0f;}\n"
    "static float tc_atan2f(float y,float x){float ax=x<0.0f?-x:x,ay=y<0.0f?-y:y,"
    "a,r,a2;int q;if(ax+ay==0.0f)return 0.0f;if(ay<ax){a=y/x;q=0;}else{a=x/y;q=2;}"
    "if(x<0.0f&&ay<ax)q=1;if(y<0.0f&&ay>=ax)q=3;a2=a*a;r=a*(1.0f-a2*(1.0f/3.0f-a2"
    "*(1.0f/5.0f-a2*(1.0f/7.0f-a2*(1.0f/9.0f)))));if(q==1)r=(r<0.0f?-3.1415926535"
    "89793f:3.141592653589793f)+r;else if(q==2)r=1.5707963267948966f-r;else if(q="
    "=3)r=-1.5707963267948966f-r;return r;}\n"
    "static float tc_rm_hue(float r,float g,float b,float iw){float a=2.0f*r-(g+b)"
    ",bb=1.7320508075688772f*(g-b),hue=tc_atan2f(bb,a),knot=hue*iw+2.0f;int j=(int"
    ")knot;static const float M[4][4]={{0.25f,0,0,0},{-0.75f,0.75f,0.75f,0.25f},{0"
    ".75f,-1.5f,0,1.0f},{-0.25f,0.75f,-0.75f,0.25f}};if(j<0||j>=4)return 0.0f;{floa"
    "t t=knot-(float)j;const float*c=M[j];return c[3]+t*(c[2]+t*(c[1]+t*c[0]));}}\n"
    "static float tc_rm_sat(float r,float g,float b){float mn=r<g?r:g,mx=r>g?r:g;"
    "mn=mn<b?mn:b;mx=mx>b?mx:b;return((mx>1e-10f?mx:1e-10f)-(mn>1e-10f?mn:1e-10f))/"
    "(mx>1e-2f?mx:1e-2f);}\n"
    "static void tc_redmod(float*px,float scale,float pivot,float iw,int inv){floa"
    "t r=px[0],g=px[1],b=px[2],fH=tc_rm_hue(r,g,b,iw);if(fH>0.0f){float nr,oms=1.0"
    "f-scale;if(inv){float mc=g<b?g:b,aa=fH*oms-1.0f,bb=r-fH*(pivot+mc)*oms,cc=fH*"
    "pivot*mc*oms,disc=bb*bb-4.0f*aa*cc;nr=disc>=0.0f?(-bb-tc_sqrtf(disc))/(2.0f*aa"
    "):r;}else{float fS=tc_rm_sat(r,g,b);nr=r+fH*fS*(pivot-r)*oms;}if(g>=b){float "
    "hf=(g-b)/((r-b)>1e-10f?(r-b):1e-10f);g=hf*(nr-b)+b;}else{float hf=(b-g)/((r-g)"
    ">1e-10f?(r-g):1e-10f);b=hf*(nr-g)+g;}r=nr;}px[0]=r;px[1]=g;px[2]=b;}\n"
    "static float tc_gc_one(float dist,float thr,float scale,float power,int inv){"
    "float ip=1.0f/power,nd,p;if(inv){if(dist>=thr+scale)return dist;nd=(dist-thr)"
    "/scale;p=tc_powf(nd,power);return thr+scale*tc_powf(-(p/(p-1.0f)),ip);}nd=(di"
    "st-thr)/scale;p=tc_powf(nd,power);return thr+scale*nd/tc_powf(1.0f+p,ip);}\n"
    "static float tc_gc_apply(float v,float ach,float thr,float scale,float power,"
    "int inv){float dist,af;if(ach==0.0f)return 0.0f;af=ach<0.0f?-ach:ach;dist=(ac"
    "h-v)/af;if(dist<thr)return v;return ach-tc_gc_one(dist,thr,scale,power,inv)*af"
    ";}\n"
    "static void tc_gamutcomp(float*px,float tC,float tM,float tY,float sC,float sM"
    ",float sY,float pw,int inv){float r=px[0],g=px[1],b=px[2],ach=r>g?r:g;ach=ach>"
    "b?ach:b;px[0]=tc_gc_apply(r,ach,tC,sC,pw,inv);px[1]=tc_gc_apply(g,ach,tM,sM,pw"
    ",inv);px[2]=tc_gc_apply(b,ach,tY,sY,pw,inv);}\n";

/* gamut-compression scale (codegen-time constant; matches interpreter gc_scale). */
static float cg_gc_scale(float lim, float thr, float power) {
    float ip, t, d;
    if (lim <= 1.0f || thr >= 1.0f) return 1.0f;
    ip = 1.0f / power;
    t = (lim - thr) / (1.0f - thr);
    d = toc_powf(toc_powf(t, power) - 1.0f, ip);
    return (d != 0.0f) ? (lim - thr) / d : 1.0f;
}

toc_result toc_emit_c(const toc_op_list *ops, const toc_codegen_c_opts *opts,
                      const toc_allocator *a, char **out_src, size_t *out_len) {
    toc_sb sb;
    const char *fname = (opts && opts->func_name) ? opts->func_name : "tocio_apply";
    int need_math = 0, need_lut1d = 0, need_lut3d = 0, lut_idx = 0;
    int need_glow = 0, need_darktodim = 0, need_rgbhsv = 0, need_xyz = 0;
    int need_hdr = 0, need_aces = 0;
    size_t k;
    if (!ops || !out_src) return TOC_ERROR_INVALID_ARGUMENT;
    if (!a) a = toc_default_allocator();
    for (k = 0; k < ops->count; ++k) {
        switch (ops->ops[k].kind) {
            case TOC_OP_EXPONENT: case TOC_OP_EXP_LINEAR: case TOC_OP_LOG:
            case TOC_OP_CDL: need_math = 1; break;
            case TOC_OP_LUT1D: need_lut1d = 1; break;
            case TOC_OP_LUT3D: need_lut3d = 1; break;
            case TOC_OP_FIXEDFUNC: {
                int s = ops->ops[k].u.fixedfunc.style;
                if (s == TOC_FF_ACES_GLOW03 || s == TOC_FF_ACES_GLOW03_INV ||
                    s == TOC_FF_ACES_GLOW10 || s == TOC_FF_ACES_GLOW10_INV)
                    need_glow = 1;
                if (s == TOC_FF_ACES_DARKTODIM10 || s == TOC_FF_ACES_DARKTODIM10_INV)
                    need_darktodim = 1;
                if (s == TOC_FF_ACES_GAMUTCOMP13 || s == TOC_FF_ACES_GAMUTCOMP13_INV)
                    need_math = need_aces = 1;
                if (s == TOC_FF_ACES_RED_MOD_03 || s == TOC_FF_ACES_RED_MOD_03_INV ||
                    s == TOC_FF_ACES_RED_MOD_10 || s == TOC_FF_ACES_RED_MOD_10_INV)
                    need_math = need_aces = 1;
                if (s == TOC_FF_RGB_TO_HSV || s == TOC_FF_HSV_TO_RGB)
                    need_rgbhsv = 1;
                if (s >= TOC_FF_XYZ_TO_xyY && s <= TOC_FF_LUV_TO_XYZ)
                    need_xyz = 1;
                if (s >= TOC_FF_LIN_TO_PQ && s <= TOC_FF_HLG_TO_LIN) {
                    need_hdr = 1; need_math = 1;
                }
                break;
            }
            default: break;
        }
    }
    toc_sb_init(&sb, a);
    toc_sb_puts(&sb,
                "/* Generated by tocio. SPDX-License-Identifier: BSD-3-Clause.\n"
                " * Color math reimplemented from OpenColorIO (BSD-3-Clause).\n"
                " */\n#include <stddef.h>\n");
    if (need_math) toc_sb_puts(&sb, MATH_SRC);
    if (need_lut1d) toc_sb_puts(&sb, LUT1D_SRC);
    if (need_lut3d) toc_sb_puts(&sb, LUT3D_SRC);
    if (need_glow) toc_sb_puts(&sb, FIXEDFUNC_SRC_GLOW);
    if (need_darktodim) toc_sb_puts(&sb, FIXEDFUNC_SRC_DARKTODIM);
    if (need_rgbhsv) toc_sb_puts(&sb, FIXEDFUNC_SRC_RGB2HSV);
    if (need_xyz) toc_sb_puts(&sb, FIXEDFUNC_SRC_XYZ);
    if (need_hdr) toc_sb_puts(&sb, FIXEDFUNC_SRC_HDR);
    if (need_aces) toc_sb_puts(&sb, FIXEDFUNC_SRC_ACES);
    /* embed LUT arrays */
    for (k = 0; k < ops->count; ++k) {
        const toc_op *op = &ops->ops[k];
        if (op->kind == TOC_OP_LUT1D)
            emit_lut_array(&sb, (int)k, op->u.lut1d.data,
                           (size_t)op->u.lut1d.length *
                               (op->u.lut1d.channels == 1 ? 1 : 3));
        else if (op->kind == TOC_OP_LUT3D)
            emit_lut_array(&sb, (int)k, op->u.lut3d.data,
                           (size_t)op->u.lut3d.size * op->u.lut3d.size *
                               op->u.lut3d.size * 3);
    }
    toc_sb_puts(&sb, "void ");
    toc_sb_puts(&sb, fname);
    toc_sb_puts(&sb, "(float *rgba, size_t npix){\n  size_t i;\n"
                     "  for(i=0;i<npix;++i){\n"
                     "  float *px=rgba+i*4;\n"
                     "  float r=px[0],g=px[1],b=px[2],a=px[3];\n");
    (void)lut_idx;
    for (k = 0; k < ops->count; ++k) {
        const toc_op *op = &ops->ops[k];
        switch (op->kind) {
            case TOC_OP_MATRIX: op_matrix(&sb, op); break;
            case TOC_OP_RANGE: op_range(&sb, op); break;
            case TOC_OP_EXPONENT: op_exponent(&sb, op); break;
            case TOC_OP_EXP_LINEAR: op_exp_linear(&sb, op); break;
            case TOC_OP_LOG: op_log(&sb, op); break;
            case TOC_OP_CDL: op_cdl(&sb, op); break;
            case TOC_OP_LUT1D:
                toc_sb_puts(&sb, "  { float c[3]={r,g,b}; tc_lut1d(tc_lut");
                toc_sb_int(&sb, (int)k);
                toc_sb_puts(&sb, ", ");
                toc_sb_int(&sb, op->u.lut1d.length);
                toc_sb_puts(&sb, ", ");
                toc_sb_int(&sb, op->u.lut1d.channels);
                toc_sb_puts(&sb, ", ");
                emit_hf(&sb, op->u.lut1d.domain_min);
                toc_sb_puts(&sb, ", ");
                emit_hf(&sb, op->u.lut1d.domain_max);
                toc_sb_puts(&sb, ", c); r=c[0];g=c[1];b=c[2]; }\n");
                break;
            case TOC_OP_LUT3D:
                toc_sb_puts(&sb, "  { float c[3]={r,g,b}; static const float dn[3]={");
                emit_hf(&sb, op->u.lut3d.domain_min[0]); toc_sb_putc(&sb, ',');
                emit_hf(&sb, op->u.lut3d.domain_min[1]); toc_sb_putc(&sb, ',');
                emit_hf(&sb, op->u.lut3d.domain_min[2]);
                toc_sb_puts(&sb, "},dx[3]={");
                emit_hf(&sb, op->u.lut3d.domain_max[0]); toc_sb_putc(&sb, ',');
                emit_hf(&sb, op->u.lut3d.domain_max[1]); toc_sb_putc(&sb, ',');
                emit_hf(&sb, op->u.lut3d.domain_max[2]);
                toc_sb_puts(&sb, "}; tc_lut3d(tc_lut");
                toc_sb_int(&sb, (int)k);
                toc_sb_puts(&sb, ", ");
                toc_sb_int(&sb, op->u.lut3d.size);
                toc_sb_puts(&sb, ", dn, dx, ");
                toc_sb_int(&sb,
                           op->u.lut3d.interp == TOC_INTERP_TETRAHEDRAL ? 1 : 0);
                toc_sb_puts(&sb, ", c); r=c[0];g=c[1];b=c[2]; }\n");
                break;
            case TOC_OP_FIXEDFUNC: {
                int s = op->u.fixedfunc.style;
                int n = op->u.fixedfunc.nparams;
                if (s == TOC_FF_REC2100_SURROUND ||
                    s == TOC_FF_REC2100_SURROUND_INV) {
                    float g = op->u.fixedfunc.params[0];
                    float Yc[3] = {0.2627f, 0.6780f, 0.0593f};
                    int inv = (s == TOC_FF_REC2100_SURROUND_INV);
                    float e = inv ? (1.0f / g - 1.0f) : (g - 1.0f);
                    toc_sb_puts(&sb, "  { float Y=");
                    emit_hf(&sb, Yc[0]); toc_sb_puts(&sb, "*r+");
                    emit_hf(&sb, Yc[1]); toc_sb_puts(&sb, "*g+");
                    emit_hf(&sb, Yc[2]); toc_sb_puts(&sb, "*b;");
                    toc_sb_puts(&sb, "if(Y>0.0f){float s=tc_powf(Y,");
                    emit_hf(&sb, e);
                    toc_sb_puts(&sb, ");r*=s;g*=s;b*=s;}}\n");
                } else if (n >= 2 &&
                    (s == TOC_FF_ACES_GLOW03 ||
                     s == TOC_FF_ACES_GLOW03_INV ||
                     s == TOC_FF_ACES_GLOW10 ||
                     s == TOC_FF_ACES_GLOW10_INV)) {
                    int inv = (s == TOC_FF_ACES_GLOW03_INV ||
                               s == TOC_FF_ACES_GLOW10_INV);
                    float gg = op->u.fixedfunc.params[0];
                    float gm = op->u.fixedfunc.params[1];
                    toc_sb_puts(&sb, "  { float c[3]={r,g,b};tc_glow(c,");
                    emit_hf(&sb, gg); toc_sb_putc(&sb, ',');
                    emit_hf(&sb, gm); toc_sb_puts(&sb, inv ? ",1);" : ",0);");
                    toc_sb_puts(&sb, " r=c[0];g=c[1];b=c[2];}\n");
                } else if (s == TOC_FF_ACES_DARKTODIM10 ||
                           s == TOC_FF_ACES_DARKTODIM10_INV) {
                    float g = op->u.fixedfunc.params[0];
                    int inv = (s == TOC_FF_ACES_DARKTODIM10_INV);
                    float gamma = inv ? (1.0f / g) : g;
                    toc_sb_puts(&sb, "  { float c[3]={r,g,b};tc_darktodim(c,");
                    emit_hf(&sb, gamma);
                    toc_sb_puts(&sb, "); r=c[0];g=c[1];b=c[2];}\n");
                } else if (n >= 6 &&
                    (s == TOC_FF_ACES_GAMUTCOMP13 ||
                     s == TOC_FF_ACES_GAMUTCOMP13_INV)) {
                    const float *pp = op->u.fixedfunc.params;
                    float pw = pp[6];
                    int inv = (s == TOC_FF_ACES_GAMUTCOMP13_INV);
                    toc_sb_puts(&sb, "  { float c[3]={r,g,b};tc_gamutcomp(c,");
                    emit_hf(&sb, pp[3]); toc_sb_putc(&sb, ',');   /* thr C/M/Y */
                    emit_hf(&sb, pp[4]); toc_sb_putc(&sb, ',');
                    emit_hf(&sb, pp[5]); toc_sb_putc(&sb, ',');
                    emit_hf(&sb, cg_gc_scale(pp[0], pp[3], pw)); toc_sb_putc(&sb, ',');
                    emit_hf(&sb, cg_gc_scale(pp[1], pp[4], pw)); toc_sb_putc(&sb, ',');
                    emit_hf(&sb, cg_gc_scale(pp[2], pp[5], pw)); toc_sb_putc(&sb, ',');
                    emit_hf(&sb, pw); toc_sb_puts(&sb, inv ? ",1);" : ",0);");
                    toc_sb_puts(&sb, " r=c[0];g=c[1];b=c[2];}\n");
                } else if (s == TOC_FF_ACES_RED_MOD_03 ||
                           s == TOC_FF_ACES_RED_MOD_03_INV ||
                           s == TOC_FF_ACES_RED_MOD_10 ||
                           s == TOC_FF_ACES_RED_MOD_10_INV) {
                    int is03 = (s == TOC_FF_ACES_RED_MOD_03 ||
                                s == TOC_FF_ACES_RED_MOD_03_INV);
                    int inv = (s == TOC_FF_ACES_RED_MOD_03_INV ||
                               s == TOC_FF_ACES_RED_MOD_10_INV);
                    toc_sb_puts(&sb, "  { float c[3]={r,g,b};tc_redmod(c,");
                    emit_hf(&sb, is03 ? 0.85f : 0.82f); toc_sb_puts(&sb, ",0.03f,");
                    emit_hf(&sb, is03 ? 1.9098593171027443f : 1.6976527263135504f);
                    toc_sb_puts(&sb, inv ? ",1);" : ",0);");
                    toc_sb_puts(&sb, " r=c[0];g=c[1];b=c[2];}\n");
                } else if (s == TOC_FF_RGB_TO_HSV ||
                           s == TOC_FF_HSV_TO_RGB) {
                    toc_sb_puts(&sb, "  { float c[3]={r,g,b};");
                    toc_sb_puts(&sb, s == TOC_FF_RGB_TO_HSV ?
                        "tc_rgb2hsv(c);" : "tc_hsv2rgb(c);");
                    toc_sb_puts(&sb, " r=c[0];g=c[1];b=c[2];}\n");
                } else if (s >= TOC_FF_XYZ_TO_xyY &&
                           s <= TOC_FF_LUV_TO_XYZ) {
                    static const char *fn[4][2] = {
                        {"tc_xyz2xyy(c)","tc_xyy2xyz(c)"},
                        {"tc_xyz2uvy(c)","tc_uvy2xyz(c)"},
                        {"tc_xyz2luv(c)","tc_luv2xyz(c)"},
                        {"tc_luv2xyz(c)","tc_xyz2luv(c)"}};
                    int idx = (s == TOC_FF_XYZ_TO_xyY ||
                               s == TOC_FF_xyY_TO_XYZ) ? 0 :
                              (s == TOC_FF_XYZ_TO_uvY ||
                               s == TOC_FF_uvY_TO_XYZ) ? 1 :
                              (s == TOC_FF_XYZ_TO_LUV ||
                               s == TOC_FF_LUV_TO_XYZ) ? 2 : 0;
                    int inv = (s == TOC_FF_xyY_TO_XYZ ||
                               s == TOC_FF_uvY_TO_XYZ ||
                               s == TOC_FF_LUV_TO_XYZ) ? 1 : 0;
                    toc_sb_puts(&sb, "  { float c[3]={r,g,b};");
                    toc_sb_puts(&sb, fn[idx][inv]);
                    toc_sb_puts(&sb, "; r=c[0];g=c[1];b=c[2];}\n");
                } else if (s >= TOC_FF_LIN_TO_PQ && s <= TOC_FF_HLG_TO_LIN) {
                    const char *fn = s == TOC_FF_LIN_TO_PQ ? "tc_pq_enc"
                                   : s == TOC_FF_PQ_TO_LIN ? "tc_pq_dec"
                                   : s == TOC_FF_LIN_TO_HLG ? "tc_hlg_enc"
                                                            : "tc_hlg_dec";
                    toc_sb_puts(&sb, "  r=");  toc_sb_puts(&sb, fn);
                    toc_sb_puts(&sb, "(r); g="); toc_sb_puts(&sb, fn);
                    toc_sb_puts(&sb, "(g); b="); toc_sb_puts(&sb, fn);
                    toc_sb_puts(&sb, "(b);\n");
                }
                break;
            }
            default: break;
        }
    }
    toc_sb_puts(&sb, "  px[0]=r;px[1]=g;px[2]=b;px[3]=a;\n  }\n}\n");
    if (sb.oom) { toc_sb_free(&sb); return TOC_ERROR_OUT_OF_MEMORY; }
    *out_src = toc_sb_take(&sb, out_len);
    return *out_src ? TOC_SUCCESS : TOC_ERROR_OUT_OF_MEMORY;
}
