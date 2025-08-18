// RUN: %dxc -T cs_6_9 -enable-16bit-types -DNUM=13   %s | FileCheck %s

// Source for dxilgen test CodeGenDXIL/passes/longvec-intrinsics.ll.
// Some targetted filecheck testing as an incidental.

RWStructuredBuffer<vector<float16_t, NUM> > hBuf;
RWStructuredBuffer<vector<float, NUM> > fBuf;
RWStructuredBuffer<vector<double, NUM> > dBuf;

RWStructuredBuffer<vector<bool, NUM> > bBuf;
RWStructuredBuffer<vector<uint, NUM> > uBuf;
RWStructuredBuffer<vector<int64_t, NUM> > lBuf;

[numthreads(8,1,1)]
void main() {

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13f32 @dx.op.rawBufferVectorLoad.v13f32(i32 303, %dx.types.Handle {{%.*}}, i32 11, i32 0, i32 4) 
  // CHECK: [[fvec1:%.*]] = extractvalue %dx.types.ResRet.v13f32 [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13f32 @dx.op.rawBufferVectorLoad.v13f32(i32 303, %dx.types.Handle {{%.*}}, i32 12, i32 0, i32 4) 
  // CHECK: [[fvec2:%.*]] = extractvalue %dx.types.ResRet.v13f32 [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13f32 @dx.op.rawBufferVectorLoad.v13f32(i32 303, %dx.types.Handle {{%.*}}, i32 13, i32 0, i32 4) 
  // CHECK: [[fvec3:%.*]] = extractvalue %dx.types.ResRet.v13f32 [[ld]], 0
  vector<float, NUM> fVec1 = fBuf[11];
  vector<float, NUM> fVec2 = fBuf[12];
  vector<float, NUM> fVec3 = fBuf[13];
  
  // CHECK: [[tmp:%.*]] = call <13 x float> @dx.op.binary.v13f32(i32 35, <13 x float> [[fvec1]], <13 x float> [[fvec2]])  ; FMax(a,b)
  // CHECK: call <13 x float> @dx.op.binary.v13f32(i32 36, <13 x float> [[tmp]], <13 x float> [[fvec3]])  ; FMin(a,b)
  vector<float, NUM> fRes = clamp(fVec1, fVec2, fVec3);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13f16 @dx.op.rawBufferVectorLoad.v13f16(i32 303, %dx.types.Handle {{%.*}}, i32 14, i32 0, i32 2) 
  // CHECK: [[hvec1:%.*]] = extractvalue %dx.types.ResRet.v13f16 [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13f16 @dx.op.rawBufferVectorLoad.v13f16(i32 303, %dx.types.Handle {{%.*}}, i32 15, i32 0, i32 2) 
  // CHECK: [[hvec2:%.*]] = extractvalue %dx.types.ResRet.v13f16 [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13f16 @dx.op.rawBufferVectorLoad.v13f16(i32 303, %dx.types.Handle {{%.*}}, i32 16, i32 0, i32 2) 
  // CHECK: [[hvec3:%.*]] = extractvalue %dx.types.ResRet.v13f16 [[ld]], 0
  vector<float16_t, NUM> hVec1 = hBuf[14];
  vector<float16_t, NUM> hVec2 = hBuf[15];
  vector<float16_t, NUM> hVec3 = hBuf[16];

  // CHECK: [[tmp:%.*]] = fcmp fast olt <13 x half> [[hvec2]], [[hvec1]]
  // CHECK: select <13 x i1> [[tmp]], <13 x half> zeroinitializer, <13 x half> <half 0xH3C00
  vector<float16_t, NUM> hRes = step(hVec1, hVec2);

  // CHECK: [[tmp:%.*]] = fmul fast <13 x float> [[fvec1]], <float 0x
  // CHECK: call <13 x float> @dx.op.unary.v13f32(i32 21, <13 x float> [[tmp]])  ; Exp(value)
  fRes += exp(fVec1);

  // CHECK: [[tmp:%.*]] = call <13 x half> @dx.op.unary.v13f16(i32 23, <13 x half> [[hvec1]])  ; Log(value)
  // CHECK: fmul fast <13 x half> [[tmp]], <half 0xH398C
  hRes += log(hVec1);

  // CHECK: [[sub:%.*]] = fsub fast <13 x float> [[fvec2]], [[fvec1]]
  // CHECK: [[xsub:%.*]] = fsub fast <13 x float> [[fvec3]], [[fvec1]]
  // CHECK: [[div:%.*]] = fdiv fast <13 x float> [[xsub]], [[sub]]
  // CHECK: [[sat:%.*]] = call <13 x float> @dx.op.unary.v13f32(i32 7, <13 x float> [[div]])  ; Saturate(value)
  // CHECK: [[mul:%.*]] = fmul fast <13 x float> [[sat]], <float 2.000000e+00,
  // CHECK: [[sub:%.*]] = fsub fast <13 x float> <float 3.000000e+00, {{.*}}>, [[mul]]
  // CHECK: [[mul:%.*]] = fmul fast <13 x float> [[sat]], [[sat]]
  // CHECK: fmul fast <13 x float> [[mul]], [[sub]]
  fRes += smoothstep(fVec1, fVec2, fVec3);

  // CHECK: fmul fast <13 x float> [[fvec3]], <float 0x3F91DF46A0000000
  fRes += radians(fVec3);

  // CHECK: [[cmp:%.*]] = fcmp fast une <13 x float> [[fvec1]], zeroinitializer
  // CHECK: [[f2i:%.*]] = bitcast <13 x float> [[fvec1]] to <13 x i32>
  // CHECK: [[and:%.*]] = and <13 x i32> [[f2i]], <i32 2139095040
  // CHECK: [[add:%.*]] = add nsw <13 x i32> [[and]], <i32 -1056964608
  // CHECK: [[shr:%.*]] = ashr <13 x i32> [[add]], <i32 23
  // CHECK: [[i2f:%.*]] = sitofp <13 x i32> [[shr]] to <13 x float>
  // CHECK: [[sel:%.*]] = select <13 x i1> [[cmp]], <13 x float> [[i2f]], <13 x float> zeroinitializer
  // CHECK: [[and:%.*]] = and <13 x i32> [[f2i]], <i32 8388607
  // CHECK: or <13 x i32> [[and]], <i32 1056964608
  vector<float, NUM> exp = fVec3;
  fRes += frexp(fVec1, exp);
  fRes += exp;

  // CHECK: [[tmp:%.*]] = fsub fast <13 x half> [[hvec3]], [[hvec2]]
  // CHECK: fmul fast <13 x half> [[tmp]], [[hvec1]]
  hRes += lerp(hVec2, hVec3, hVec1);

  // CHECK: [[tmp:%.*]] = call <13 x float> @dx.op.unary.v13f32(i32 83, <13 x float> [[fvec1]])  ; DerivCoarseX(value)
  // CHECK: call <13 x float> @dx.op.unary.v13f32(i32 6, <13 x float> [[tmp]])  ; FAbs(value)
  // CHECK: [[tmp:%.*]] = call <13 x float> @dx.op.unary.v13f32(i32 84, <13 x float> [[fvec1]])  ; DerivCoarseY(value)
  // CHECK: call <13 x float> @dx.op.unary.v13f32(i32 6, <13 x float> [[tmp]])  ; FAbs(value)
  fRes += fwidth(fVec1);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13i32 @dx.op.rawBufferVectorLoad.v13i32(i32 303, %dx.types.Handle {{%.*}}, i32 17, i32 0, i32 4) 
  // CHECK: [[uvec1:%.*]] = extractvalue %dx.types.ResRet.v13i32 [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13i32 @dx.op.rawBufferVectorLoad.v13i32(i32 303, %dx.types.Handle {{%.*}}, i32 18, i32 0, i32 4) 
  // CHECK: [[uvec2:%.*]] = extractvalue %dx.types.ResRet.v13i32 [[ld]], 0
  vector<uint, NUM> uVec1 = uBuf[17];
  vector<uint, NUM> uVec2 = uBuf[18];

  vector<uint, NUM> signs = 1;
  // CHECK: [[cmp:%.*]] = icmp ne <13 x i32> [[uvec2]], zeroinitializer
  // CHECK: zext <13 x i1> [[cmp]] to <13 x i32>
  signs *= sign(uVec2);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13i64 @dx.op.rawBufferVectorLoad.v13i64(i32 303, %dx.types.Handle {{%.*}}, i32 19, i32 0, i32 8) 
  // CHECK: [[lvec1:%.*]] = extractvalue %dx.types.ResRet.v13i64 [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13i64 @dx.op.rawBufferVectorLoad.v13i64(i32 303, %dx.types.Handle {{%.*}}, i32 20, i32 0, i32 8) 
  // CHECK: [[lvec2:%.*]] = extractvalue %dx.types.ResRet.v13i64 [[ld]], 0
  vector<int64_t, NUM> lVec1 = lBuf[19];
  vector<int64_t, NUM> lVec2 = lBuf[20];

  // CHECK: [[gt:%.*]] = icmp sgt <13 x i64> [[lvec2]], zeroinitializer
  // CHECK: [[lt:%.*]] = icmp slt <13 x i64> [[lvec2]], zeroinitializer
  // CHECK: [[igt:%.*]] = zext <13 x i1> [[gt]] to <13 x i32>
  // CHECK: [[ilt:%.*]] = zext <13 x i1> [[lt]] to <13 x i32>
  // CHECK: sub nsw <13 x i32> [[igt]], [[ilt]]
  signs *= sign(lVec2);

  vector<uint, NUM> uRes = signs;

  // CHECK: call <13 x i32> @dx.op.unaryBits.v13i32(i32 31, <13 x i32> [[uvec2]])  ; Countbits(value)
  uRes += countbits(uVec2);

  // CHECK: call <13 x i32> @dx.op.unaryBits.v13i64(i32 32, <13 x i64> [[lvec2]])  ; FirstbitLo(value)
  uRes += firstbitlow(lVec2);

  // CHECK: [[bit:%.*]] = call <13 x i32> @dx.op.unaryBits.v13i32(i32 33, <13 x i32> [[uvec1]])  ; FirstbitHi(value)
  // CHECK: sub <13 x i32> <i32 31, {{.*}}>, [[bit]]
  // CHECK: icmp eq <13 x i32> [[bit]], <i32 -1,
  uRes += firstbithigh(uVec1);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13i32 @dx.op.rawBufferVectorLoad.v13i32(i32 303, %dx.types.Handle {{%.*}}, i32 21, i32 0, i32 4) 
  // CHECK: [[vec:%.*]] = extractvalue %dx.types.ResRet.v13i32 [[ld]], 0
  // CHECK: [[bvec:%.*]] = icmp ne <13 x i32> [[vec]], zeroinitializer
  // CHECK: [[vec1:%.*]] = zext <13 x i1> [[bvec]] to <13 x i32>
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13i32 @dx.op.rawBufferVectorLoad.v13i32(i32 303, %dx.types.Handle {{%.*}}, i32 22, i32 0, i32 4) 
  // CHECK: [[vec:%.*]] = extractvalue %dx.types.ResRet.v13i32 [[ld]], 0
  // CHECK: [[bvec:%.*]] = icmp ne <13 x i32> [[vec]], zeroinitializer
  // CHECK: [[vec2:%.*]] = zext <13 x i1> [[bvec]] to <13 x i32>
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13i32 @dx.op.rawBufferVectorLoad.v13i32(i32 303, %dx.types.Handle {{%.*}}, i32 23, i32 0, i32 4) 
  // CHECK: [[vec:%.*]] = extractvalue %dx.types.ResRet.v13i32 [[ld]], 0
  // CHECK: [[bvec:%.*]] = icmp ne <13 x i32> [[vec]], zeroinitializer
  // CHECK: [[vec3:%.*]] = zext <13 x i1> [[bvec]] to <13 x i32>
  vector<bool, NUM> bVec1 = bBuf[21];
  vector<bool, NUM> bVec2 = bBuf[22];
  vector<bool, NUM> bVec3 = bBuf[23];

  // CHECK: [[bvec2:%.*]] = icmp ne <13 x i32> [[vec2]], zeroinitializer
  // CHECK: [[bvec1:%.*]] = icmp ne <13 x i32> [[vec1]], zeroinitializer
  // CHECK: or <13 x i1> [[bvec2]], [[bvec1]]
  uRes += or(bVec1, bVec2);

  // CHECK: [[bvec3:%.*]] = icmp ne <13 x i32> [[vec3]], zeroinitializer
  // CHECK: and <13 x i1> [[bvec3]], [[bvec2]]
  uRes += and(bVec2, bVec3);

  // CHECK: select <13 x i1> [[bvec3]], <13 x i64> [[lvec1]], <13 x i64> [[lvec2]]
  vector<int64_t, NUM> lRes = select(bVec3, lVec1, lVec2);

  // CHECK: call <13 x i1> @dx.op.isSpecialFloat.v13f32(i32 8, <13 x float> [[fvec2]])  ; IsNaN(value)
  uRes += isnan(fVec2);

  // CHECK: [[el1:%.*]] = extractelement <13 x float> [[fvec1]]
  // CHECK: [[el2:%.*]] = extractelement <13 x float> [[fvec2]]
  // CHECK: [[mul:%.*]] = fmul fast float [[el2]], [[el1]]
  // CHECK: [[mad1:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mul]]) ; FMad(a,b,c)
  // CHECK: [[mad2:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad1]]) ; FMad(a,b,c)
  // CHECK: [[mad3:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad2]]) ; FMad(a,b,c)
  // CHECK: [[mad4:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad3]]) ; FMad(a,b,c)
  // CHECK: [[mad5:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad4]]) ; FMad(a,b,c)
  // CHECK: [[mad6:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad5]]) ; FMad(a,b,c)
  // CHECK: [[mad7:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad6]]) ; FMad(a,b,c)
  // CHECK: [[mad8:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad7]]) ; FMad(a,b,c)
  // CHECK: [[mad9:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad8]]) ; FMad(a,b,c)
  // CHECK: [[mad10:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad9]]) ; FMad(a,b,c)
  // CHECK: [[mad11:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad10]]) ; FMad(a,b,c)
  // CHECK: [[mad12:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mad11]]) ; FMad(a,b,c)
  fRes += dot(fVec1, fVec2);

  // CHECK: call <13 x float> @dx.op.unary.v13f32(i32 17, <13 x float> [[fvec1]])  ; Atan(value)
  fRes += atan(fVec1);

  // CHECK: call <13 x i32> @dx.op.binary.v13i32(i32 40, <13 x i32> [[uvec1]], <13 x i32> [[uvec2]])  ; UMin(a,b)
  uRes += min(uVec1, uVec2);

  // CHECK: call <13 x float> @dx.op.tertiary.v13f32(i32 46, <13 x float> [[fvec1]], <13 x float> [[fvec2]], <13 x float> [[fvec3]])  ; FMad(a,b,c)
  fRes += mad(fVec1, fVec2, fVec3);

  // CHECK: call <13 x half> @dx.op.unary.v13f16(i32 85, <13 x half> [[hvec1]])  ; DerivFineX(value)
  hRes += ddx_fine(hVec1);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13f64 @dx.op.rawBufferVectorLoad.v13f64(i32 303, %dx.types.Handle {{%.*}}, i32 24, i32 0, i32 8) 
  // CHECK: [[dvec1:%.*]] = extractvalue %dx.types.ResRet.v13f64 [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13f64 @dx.op.rawBufferVectorLoad.v13f64(i32 303, %dx.types.Handle {{%.*}}, i32 25, i32 0, i32 8) 
  // CHECK: [[dvec2:%.*]] = extractvalue %dx.types.ResRet.v13f64 [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.v13f64 @dx.op.rawBufferVectorLoad.v13f64(i32 303, %dx.types.Handle {{%.*}}, i32 26, i32 0, i32 8) 
  // CHECK: [[dvec3:%.*]] = extractvalue %dx.types.ResRet.v13f64 [[ld]], 0
  vector<double, NUM> dVec1 = dBuf[24];
  vector<double, NUM> dVec2 = dBuf[25];
  vector<double, NUM> dVec3 = dBuf[26];

  // CHECK: call <13 x double> @dx.op.tertiary.v13f64(i32 47, <13 x double> [[dvec1]], <13 x double> [[dvec2]], <13 x double> [[dvec3]])
  vector<double, NUM> dRes = fma(dVec1, dVec2, dVec3);

  hBuf[0] = hRes;
  fBuf[0] = fRes;
  dBuf[0] = dRes;
  uBuf[0] = uRes;
  lBuf[0] = lRes;
}
