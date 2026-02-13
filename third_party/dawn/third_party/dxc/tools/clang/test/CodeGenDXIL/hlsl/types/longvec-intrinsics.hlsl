// RUN: %dxc -T cs_6_9 -enable-16bit-types -DNUM=2   %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DNUM=7   %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DNUM=125 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DNUM=256 %s | FileCheck %s

// Test vector-enabled non-trivial intrinsics that take parameters of various types.

RWByteAddressBuffer buf;
RWByteAddressBuffer ibuf;

// CHECK-DAG: %dx.types.ResRet.[[STY:v[0-9]*i16]] = type { <[[NUM:[0-9]*]] x i16>
// CHECK-DAG: %dx.types.ResRet.[[ITY:v[0-9]*i32]] = type { <[[NUM]] x i32>
// CHECK-DAG: %dx.types.ResRet.[[LTY:v[0-9]*i64]] = type { <[[NUM]] x i64>

// CHECK-DAG: %dx.types.ResRet.[[HTY:v[0-9]*f16]] = type { <[[NUM:[0-9]*]] x half>
// CHECK-DAG: %dx.types.ResRet.[[FTY:v[0-9]*f32]] = type { <[[NUM]] x float>
// CHECK-DAG: %dx.types.ResRet.[[DTY:v[0-9]*f64]] = type { <[[NUM]] x double>

[numthreads(8,1,1)]
void main() {
  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle {{%.*}}, %dx.types.ResourceProperties { i32 4107, i32 0 })

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[HTY]] @dx.op.rawBufferVectorLoad.[[HTY]](i32 303, %dx.types.Handle [[buf]], i32 0
  // CHECK: [[hvec1:%.*]] = extractvalue %dx.types.ResRet.[[HTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[HTY]] @dx.op.rawBufferVectorLoad.[[HTY]](i32 303, %dx.types.Handle [[buf]], i32 512
  // CHECK: [[hvec2:%.*]] = extractvalue %dx.types.ResRet.[[HTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[HTY]] @dx.op.rawBufferVectorLoad.[[HTY]](i32 303, %dx.types.Handle [[buf]], i32 1024
  // CHECK: [[hvec3:%.*]] = extractvalue %dx.types.ResRet.[[HTY]] [[ld]], 0
  vector<float16_t, NUM> hVec1 = buf.Load<vector<float16_t, NUM> >(0);
  vector<float16_t, NUM> hVec2 = buf.Load<vector<float16_t, NUM> >(512);
  vector<float16_t, NUM> hVec3 = buf.Load<vector<float16_t, NUM> >(1024);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[FTY]] @dx.op.rawBufferVectorLoad.[[FTY]](i32 303, %dx.types.Handle [[buf]], i32 2048
  // CHECK: [[fvec1:%.*]] = extractvalue %dx.types.ResRet.[[FTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[FTY]] @dx.op.rawBufferVectorLoad.[[FTY]](i32 303, %dx.types.Handle [[buf]], i32 2560
  // CHECK: [[fvec2:%.*]] = extractvalue %dx.types.ResRet.[[FTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[FTY]] @dx.op.rawBufferVectorLoad.[[FTY]](i32 303, %dx.types.Handle [[buf]], i32 3072
  // CHECK: [[fvec3:%.*]] = extractvalue %dx.types.ResRet.[[FTY]] [[ld]], 0
  vector<float, NUM> fVec1 = buf.Load<vector<float, NUM> >(2048);
  vector<float, NUM> fVec2 = buf.Load<vector<float, NUM> >(2560);
  vector<float, NUM> fVec3 = buf.Load<vector<float, NUM> >(3072);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[DTY]] @dx.op.rawBufferVectorLoad.[[DTY]](i32 303, %dx.types.Handle [[buf]], i32 4096
  // CHECK: [[dvec1:%.*]] = extractvalue %dx.types.ResRet.[[DTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[DTY]] @dx.op.rawBufferVectorLoad.[[DTY]](i32 303, %dx.types.Handle [[buf]], i32 4608
  // CHECK: [[dvec2:%.*]] = extractvalue %dx.types.ResRet.[[DTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[DTY]] @dx.op.rawBufferVectorLoad.[[DTY]](i32 303, %dx.types.Handle [[buf]], i32 5120
  // CHECK: [[dvec3:%.*]] = extractvalue %dx.types.ResRet.[[DTY]] [[ld]], 0
  vector<double, NUM> dVec1 = buf.Load<vector<double, NUM> >(4096);
  vector<double, NUM> dVec2 = buf.Load<vector<double, NUM> >(4608);
  vector<double, NUM> dVec3 = buf.Load<vector<double, NUM> >(5120);

  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle {{%.*}}, %dx.types.ResourceProperties { i32 4107, i32 0 })

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferVectorLoad.[[STY]](i32 303, %dx.types.Handle [[buf]], i32 0
  // CHECK: [[svec1:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferVectorLoad.[[STY]](i32 303, %dx.types.Handle [[buf]], i32 512
  // CHECK: [[svec2:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferVectorLoad.[[STY]](i32 303, %dx.types.Handle [[buf]], i32 1024
  // CHECK: [[svec3:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  vector<int16_t, NUM> sVec1 = ibuf.Load<vector<int16_t, NUM> >(0);
  vector<int16_t, NUM> sVec2 = ibuf.Load<vector<int16_t, NUM> >(512);
  vector<int16_t, NUM> sVec3 = ibuf.Load<vector<int16_t, NUM> >(1024);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferVectorLoad.[[STY]](i32 303, %dx.types.Handle [[buf]], i32 1025
  // CHECK: [[usvec1:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferVectorLoad.[[STY]](i32 303, %dx.types.Handle [[buf]], i32 1536
  // CHECK: [[usvec2:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[STY]] @dx.op.rawBufferVectorLoad.[[STY]](i32 303, %dx.types.Handle [[buf]], i32 2048
  // CHECK: [[usvec3:%.*]] = extractvalue %dx.types.ResRet.[[STY]] [[ld]], 0
  vector<uint16_t, NUM> usVec1 = ibuf.Load<vector<uint16_t, NUM> >(1025);
  vector<uint16_t, NUM> usVec2 = ibuf.Load<vector<uint16_t, NUM> >(1536);
  vector<uint16_t, NUM> usVec3 = ibuf.Load<vector<uint16_t, NUM> >(2048);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[buf]], i32 2049
  // CHECK: [[ivec1:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[buf]], i32 2560
  // CHECK: [[ivec2:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[buf]], i32 3072
  // CHECK: [[ivec3:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  vector<int, NUM> iVec1 = ibuf.Load<vector<int, NUM> >(2049);
  vector<int, NUM> iVec2 = ibuf.Load<vector<int, NUM> >(2560);
  vector<int, NUM> iVec3 = ibuf.Load<vector<int, NUM> >(3072);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[buf]], i32 3073
  // CHECK: [[uivec1:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[buf]], i32 3584
  // CHECK: [[uivec2:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[ITY]] @dx.op.rawBufferVectorLoad.[[ITY]](i32 303, %dx.types.Handle [[buf]], i32 4096
  // CHECK: [[uivec3:%.*]] = extractvalue %dx.types.ResRet.[[ITY]] [[ld]], 0
  vector<uint, NUM> uiVec1 = ibuf.Load<vector<uint, NUM> >(3073);
  vector<uint, NUM> uiVec2 = ibuf.Load<vector<uint, NUM> >(3584);
  vector<uint, NUM> uiVec3 = ibuf.Load<vector<uint, NUM> >(4096);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[LTY]] @dx.op.rawBufferVectorLoad.[[LTY]](i32 303, %dx.types.Handle [[buf]], i32 4097
  // CHECK: [[lvec1:%.*]] = extractvalue %dx.types.ResRet.[[LTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[LTY]] @dx.op.rawBufferVectorLoad.[[LTY]](i32 303, %dx.types.Handle [[buf]], i32 4608
  // CHECK: [[lvec2:%.*]] = extractvalue %dx.types.ResRet.[[LTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[LTY]] @dx.op.rawBufferVectorLoad.[[LTY]](i32 303, %dx.types.Handle [[buf]], i32 5120
  // CHECK: [[lvec3:%.*]] = extractvalue %dx.types.ResRet.[[LTY]] [[ld]], 0
  vector<int64_t, NUM> lVec1 = ibuf.Load<vector<int64_t, NUM> >(4097);
  vector<int64_t, NUM> lVec2 = ibuf.Load<vector<int64_t, NUM> >(4608);
  vector<int64_t, NUM> lVec3 = ibuf.Load<vector<int64_t, NUM> >(5120);

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[LTY]] @dx.op.rawBufferVectorLoad.[[LTY]](i32 303, %dx.types.Handle [[buf]], i32 5121
  // CHECK: [[ulvec1:%.*]] = extractvalue %dx.types.ResRet.[[LTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[LTY]] @dx.op.rawBufferVectorLoad.[[LTY]](i32 303, %dx.types.Handle [[buf]], i32 5632
  // CHECK: [[ulvec2:%.*]] = extractvalue %dx.types.ResRet.[[LTY]] [[ld]], 0
  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[LTY]] @dx.op.rawBufferVectorLoad.[[LTY]](i32 303, %dx.types.Handle [[buf]], i32 6144
  // CHECK: [[ulvec3:%.*]] = extractvalue %dx.types.ResRet.[[LTY]] [[ld]], 0
  vector<uint64_t, NUM> ulVec1 = ibuf.Load<vector<uint64_t, NUM> >(5121);
  vector<uint64_t, NUM> ulVec2 = ibuf.Load<vector<uint64_t, NUM> >(5632);
  vector<uint64_t, NUM> ulVec3 = ibuf.Load<vector<uint64_t, NUM> >(6144);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x half> @dx.op.binary.[[HTY]](i32 35, <[[NUM]] x half> [[hvec1]], <[[NUM]] x half> [[hvec2]])  ; FMax(a,b)
  // CHECK: call <[[NUM]] x half> @dx.op.binary.[[HTY]](i32 36, <[[NUM]] x half> [[tmp]], <[[NUM]] x half> [[hvec3]])  ; FMin(a,b)
  vector<float16_t, NUM> hRes = clamp(hVec1, hVec2, hVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x float> @dx.op.binary.[[FTY]](i32 35, <[[NUM]] x float> [[fvec1]], <[[NUM]] x float> [[fvec2]])  ; FMax(a,b)
  // CHECK: call <[[NUM]] x float> @dx.op.binary.[[FTY]](i32 36, <[[NUM]] x float> [[tmp]], <[[NUM]] x float> [[fvec3]])  ; FMin(a,b)
  vector<float, NUM> fRes = clamp(fVec1, fVec2, fVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x double> @dx.op.binary.[[DTY]](i32 35, <[[NUM]] x double> [[dvec1]], <[[NUM]] x double> [[dvec2]])  ; FMax(a,b)
  // CHECK: call <[[NUM]] x double> @dx.op.binary.[[DTY]](i32 36, <[[NUM]] x double> [[tmp]], <[[NUM]] x double> [[dvec3]])  ; FMin(a,b)
  vector<double, NUM> dRes = clamp(dVec1, dVec2, dVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x i16> @dx.op.binary.[[STY]](i32 37, <[[NUM]] x i16> [[svec1]], <[[NUM]] x i16> [[svec2]])  ; IMax(a,b)
  // CHECK: call <[[NUM]] x i16> @dx.op.binary.[[STY]](i32 38, <[[NUM]] x i16> [[tmp]], <[[NUM]] x i16> [[svec3]])  ; IMin(a,b)
  vector<int16_t, NUM> sRes = clamp(sVec1, sVec2, sVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x i16> @dx.op.binary.[[STY]](i32 39, <[[NUM]] x i16> [[usvec1]], <[[NUM]] x i16> [[usvec2]])  ; UMax(a,b)
  // CHECK: call <[[NUM]] x i16> @dx.op.binary.[[STY]](i32 40, <[[NUM]] x i16> [[tmp]], <[[NUM]] x i16> [[usvec3]])  ; UMin(a,b)
  vector<uint16_t, NUM> usRes = clamp(usVec1, usVec2, usVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x i32> @dx.op.binary.[[ITY]](i32 37, <[[NUM]] x i32> [[ivec1]], <[[NUM]] x i32> [[ivec2]])  ; IMax(a,b)
  // CHECK: call <[[NUM]] x i32> @dx.op.binary.[[ITY]](i32 38, <[[NUM]] x i32> [[tmp]], <[[NUM]] x i32> [[ivec3]])  ; IMin(a,b)
  vector<int, NUM> iRes = clamp(iVec1, iVec2, iVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x i32> @dx.op.binary.[[ITY]](i32 39, <[[NUM]] x i32> [[uivec1]], <[[NUM]] x i32> [[uivec2]])  ; UMax(a,b)
  // CHECK: call <[[NUM]] x i32> @dx.op.binary.[[ITY]](i32 40, <[[NUM]] x i32> [[tmp]], <[[NUM]] x i32> [[uivec3]])  ; UMin(a,b)
  vector<uint, NUM> uiRes = clamp(uiVec1, uiVec2, uiVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x i64> @dx.op.binary.[[LTY]](i32 37, <[[NUM]] x i64> [[lvec1]], <[[NUM]] x i64> [[lvec2]])  ; IMax(a,b)
  // CHECK: call <[[NUM]] x i64> @dx.op.binary.[[LTY]](i32 38, <[[NUM]] x i64> [[tmp]], <[[NUM]] x i64> [[lvec3]])  ; IMin(a,b)
  vector<int64_t, NUM> lRes = clamp(lVec1, lVec2, lVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x i64> @dx.op.binary.[[LTY]](i32 39, <[[NUM]] x i64> [[ulvec1]], <[[NUM]] x i64> [[ulvec2]])  ; UMax(a,b)
  // CHECK: call <[[NUM]] x i64> @dx.op.binary.[[LTY]](i32 40, <[[NUM]] x i64> [[tmp]], <[[NUM]] x i64> [[ulvec3]])  ; UMin(a,b)
  vector<uint64_t, NUM> ulRes = clamp(ulVec1, ulVec2, ulVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = fcmp fast olt <[[NUM]] x half> [[hvec2]], [[hvec1]]
  // CHECK: select <[[NUM]] x i1> [[tmp]], <[[NUM]] x half> zeroinitializer, <[[NUM]] x half> <half 0xH3C00
  hRes += step(hVec1, hVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = fcmp fast olt <[[NUM]] x float> [[fvec2]], [[fvec1]]
  // CHECK: select <[[NUM]] x i1> [[tmp]], <[[NUM]] x float> zeroinitializer, <[[NUM]] x float> <float 1
  fRes += step(fVec1, fVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = fmul fast <[[NUM]] x half> [[hvec1]], <half 0x
  // CHECK: call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 21, <[[NUM]] x half> [[tmp]])  ; Exp(value)
  hRes += exp(hVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = fmul fast <[[NUM]] x float> [[fvec1]], <float 0x
  // CHECK: call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 21, <[[NUM]] x float> [[tmp]])  ; Exp(value)
  fRes += exp(fVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 23, <[[NUM]] x half> [[hvec1]])  ; Log(value)
  // CHECK: fmul fast <[[NUM]] x half> [[tmp]], <half 0xH398C
  hRes += log(hVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 23, <[[NUM]] x float> [[fvec1]])  ; Log(value)
  // CHECK: fmul fast <[[NUM]] x float> [[tmp]], <float 0x3FE62E4300000000
  fRes += log(fVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 23, <[[NUM]] x half> [[hvec2]])  ; Log(value)
  // CHECK: [[tmp2:%.*]] = fmul fast <[[NUM]] x half> [[tmp]], [[hvec1]]
  // CHECK: call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 21, <[[NUM]] x half> [[tmp2]])  ; Exp(value)
  hRes += pow(hVec2, hVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 23, <[[NUM]] x float> [[fvec2]])  ; Log(value)
  // CHECK: [[tmp2:%.*]] = fmul fast <[[NUM]] x float> [[tmp]], [[fvec1]]
  // CHECK: call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 21, <[[NUM]] x float> [[tmp2]])  ; Exp(value)
  fRes += pow(fVec2, fVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[div:%.*]] = fdiv fast <[[NUM]] x half> [[hvec3]], [[hvec2]]
  // CHECK: [[atan:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 17, <[[NUM]] x half> [[div]]) ; Atan(value)
  // CHECK: [[add:%.*]] = fadd fast <[[NUM]] x half> [[atan]], <half 0x
  // CHECK: [[sub:%.*]] = fsub fast <[[NUM]] x half> [[atan]], <half 0x
  // CHECK: [[xlt:%.*]] = fcmp fast olt <[[NUM]] x half> [[hvec2]], zeroinitializer
  // CHECK: [[xeq:%.*]] = fcmp fast oeq <[[NUM]] x half> [[hvec2]], zeroinitializer
  // CHECK: [[yge:%.*]] = fcmp fast oge <[[NUM]] x half> [[hvec3]], zeroinitializer
  // CHECK: [[ylt:%.*]] = fcmp fast olt <[[NUM]] x half> [[hvec3]], zeroinitializer
  // CHECK: [[and:%.*]] = and <[[NUM]] x i1> [[yge]], [[xlt]]
  // CHECK: select <[[NUM]] x i1> [[and]], <[[NUM]] x half> [[add]], <[[NUM]] x half>
  // CHECK: [[and:%.*]] = and <[[NUM]] x i1> [[ylt]], [[xlt]]
  // CHECK: select <[[NUM]] x i1> [[and]], <[[NUM]] x half> [[sub]], <[[NUM]] x half>
  // CHECK: [[and:%.*]] = and <[[NUM]] x i1> [[ylt]], [[xeq]]
  // CHECK: select <[[NUM]] x i1> [[and]], <[[NUM]] x half> <half 0x
  // CHECK: [[and:%.*]] = and <[[NUM]] x i1> [[yge]], [[xeq]]
  // CHECK: select <[[NUM]] x i1> [[and]], <[[NUM]] x half> <half 0x
  hRes += atan2(hVec3, hVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[div:%.*]] = fdiv fast <[[NUM]] x float> [[fvec3]], [[fvec2]]
  // CHECK: [[atan:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 17, <[[NUM]] x float> [[div]]) ; Atan(value)
  // CHECK: [[add:%.*]] = fadd fast <[[NUM]] x float> [[atan]], <float 0x
  // CHECK: [[sub:%.*]] = fadd fast <[[NUM]] x float> [[atan]], <float 0x
  // CHECK: [[xlt:%.*]] = fcmp fast olt <[[NUM]] x float> [[fvec2]], zeroinitializer
  // CHECK: [[xeq:%.*]] = fcmp fast oeq <[[NUM]] x float> [[fvec2]], zeroinitializer
  // CHECK: [[yge:%.*]] = fcmp fast oge <[[NUM]] x float> [[fvec3]], zeroinitializer
  // CHECK: [[ylt:%.*]] = fcmp fast olt <[[NUM]] x float> [[fvec3]], zeroinitializer
  // CHECK: [[and:%.*]] = and <[[NUM]] x i1> [[yge]], [[xlt]]
  // CHECK: select <[[NUM]] x i1> [[and]], <[[NUM]] x float> [[add]], <[[NUM]] x float>
  // CHECK: [[and:%.*]] = and <[[NUM]] x i1> [[ylt]], [[xlt]]
  // CHECK: select <[[NUM]] x i1> [[and]], <[[NUM]] x float> [[sub]], <[[NUM]] x float>
  // CHECK: [[and:%.*]] = and <[[NUM]] x i1> [[ylt]], [[xeq]]
  // CHECK: select <[[NUM]] x i1> [[and]], <[[NUM]] x float> <float 0x
  // CHECK: [[and:%.*]] = and <[[NUM]] x i1> [[yge]], [[xeq]]
  // CHECK: select <[[NUM]] x i1> [[and]], <[[NUM]] x float> <float 0x
  fRes += atan2(fVec3, fVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[div:%.*]] = fdiv fast <[[NUM]] x half> [[hvec2]], [[hvec3]]
  // CHECK: [[ndiv:%.*]] = fsub fast <[[NUM]] x half> {{.*}}, [[div]]
  // CHECK: [[cmp:%.*]] = fcmp fast oge <[[NUM]] x half> [[div]], [[ndiv]]
  // CHECK: [[abs:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 6, <[[NUM]] x half> [[div]]) ; FAbs(value)
  // CHECK: [[frc:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 22, <[[NUM]] x half> [[abs]]) ; Frc(value)
  // CHECK: [[nfrc:%.*]] = fsub fast <[[NUM]] x half> {{.*}}, [[frc]]
  // CHECK: [[rfrc:%.*]] = select <[[NUM]] x i1> [[cmp]], <[[NUM]] x half> [[frc]], <[[NUM]] x half> [[nfrc]]
  // CHECK: fmul fast <[[NUM]] x half> [[rfrc]], [[hvec3]]
  hRes += fmod(hVec2, hVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[div:%.*]] = fdiv fast <[[NUM]] x float> [[fvec2]], [[fvec3]]
  // CHECK: [[ndiv:%.*]] = fsub fast <[[NUM]] x float> {{.*}}, [[div]]
  // CHECK: [[cmp:%.*]] = fcmp fast oge <[[NUM]] x float> [[div]], [[ndiv]]
  // CHECK: [[abs:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 6, <[[NUM]] x float> [[div]]) ; FAbs(value)
  // CHECK: [[frc:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 22, <[[NUM]] x float> [[abs]]) ; Frc(value)
  // CHECK: [[nfrc:%.*]] = fsub fast <[[NUM]] x float> {{.*}}, [[frc]]
  // CHECK: [[rfrc:%.*]] = select <[[NUM]] x i1> [[cmp]], <[[NUM]] x float> [[frc]], <[[NUM]] x float> [[nfrc]]
  // CHECK: fmul fast <[[NUM]] x float> [[rfrc]], [[fvec3]]
  fRes += fmod(fVec2, fVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[exp:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 21, <[[NUM]] x half> [[hvec2]]) ; Exp(value)
  // CHECK: fmul fast <[[NUM]] x half> [[exp]], [[hvec1]]
  hRes += ldexp(hVec1, hVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[exp:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 21, <[[NUM]] x float> [[fvec2]]) ; Exp(value)
  // CHECK: fmul fast <[[NUM]] x float> [[exp]], [[fvec1]]
  fRes += ldexp(fVec1, fVec2);

  vector<half, NUM> hVal;
  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 29, <[[NUM]] x half> [[hvec1]])  ; Round_z(value)
  // CHECK: fsub fast <[[NUM]] x half> [[hvec1]], [[tmp]]
  hRes *= modf(hVec1, hVal);
  hRes += hVal;

  vector<float, NUM> fVal;
  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 29, <[[NUM]] x float> [[fvec1]])  ; Round_z(value)
  // CHECK: fsub fast <[[NUM]] x float> [[fvec1]], [[tmp]]
  fRes *= modf(fVec1, fVal);
  fRes += fVal;

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[sub:%.*]] = fsub fast <[[NUM]] x half> [[hvec2]], [[hvec1]]
  // CHECK: [[xsub:%.*]] = fsub fast <[[NUM]] x half> [[hvec3]], [[hvec1]]
  // CHECK: [[div:%.*]] = fdiv fast <[[NUM]] x half> [[xsub]], [[sub]]
  // CHECK: [[sat:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 7, <[[NUM]] x half> [[div]])  ; Saturate(value)
  // CHECK: [[mul:%.*]] = fmul fast <[[NUM]] x half> [[sat]], <half 0xH4000,
  // CHECK: [[sub:%.*]] = fsub fast <[[NUM]] x half> <half 0xH4200, {{.*}}>, [[mul]]
  // CHECK: [[mul:%.*]] = fmul fast <[[NUM]] x half> [[sat]], [[sat]]
  // CHECK: fmul fast <[[NUM]] x half> [[mul]], [[sub]]
  hRes += smoothstep(hVec1, hVec2, hVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[sub:%.*]] = fsub fast <[[NUM]] x float> [[fvec2]], [[fvec1]]
  // CHECK: [[xsub:%.*]] = fsub fast <[[NUM]] x float> [[fvec3]], [[fvec1]]
  // CHECK: [[div:%.*]] = fdiv fast <[[NUM]] x float> [[xsub]], [[sub]]
  // CHECK: [[sat:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 7, <[[NUM]] x float> [[div]])  ; Saturate(value)
  // CHECK: [[mul:%.*]] = fmul fast <[[NUM]] x float> [[sat]], <float 2.000000e+00,
  // CHECK: [[sub:%.*]] = fsub fast <[[NUM]] x float> <float 3.000000e+00, {{.*}}>, [[mul]]
  // CHECK: [[mul:%.*]] = fmul fast <[[NUM]] x float> [[sat]], [[sat]]
  // CHECK: fmul fast <[[NUM]] x float> [[mul]], [[sub]]
  fRes += smoothstep(fVec1, fVec2, fVec3);

  // Note that Fabs is tested in longvec-trivial-unary-float-intrinsics.
  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = sub <[[NUM]] x i16> zeroinitializer, [[svec1]]
  // CHECK: call <[[NUM]] x i16> @dx.op.binary.[[STY]](i32 37, <[[NUM]] x i16> [[svec1]], <[[NUM]] x i16> [[tmp]])  ; IMax(a,b)
  sRes += abs(sVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = sub <[[NUM]] x i32> zeroinitializer, [[ivec1]]
  // CHECK: call <[[NUM]] x i32> @dx.op.binary.[[ITY]](i32 37, <[[NUM]] x i32> [[ivec1]], <[[NUM]] x i32> [[tmp]])  ; IMax(a,b)
  iRes += abs(iVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = sub <[[NUM]] x i64> zeroinitializer, [[lvec1]]
  // CHECK: call <[[NUM]] x i64> @dx.op.binary.[[LTY]](i32 37, <[[NUM]] x i64> [[lvec1]], <[[NUM]] x i64> [[tmp]])  ; IMax(a,b)
  lRes += abs(lVec1);

  // Intrinsics that expand into llvm ops.

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: fmul fast <[[NUM]] x half> [[hvec2]], <half 0xH5329
  hRes += degrees(hVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: fmul fast <[[NUM]] x float> [[fvec2]], <float 0x404CA5DC20000000
  fRes += degrees(fVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: fmul fast <[[NUM]] x half> [[hvec3]], <half 0xH2478
  hRes += radians(hVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: fmul fast <[[NUM]] x float> [[fvec3]], <float 0x3F91DF46A0000000
  fRes += radians(fVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[cmp:%.*]] = fcmp fast une <[[NUM]] x float> [[fvec1]], zeroinitializer
  // CHECK: [[f2i:%.*]] = bitcast <[[NUM]] x float> [[fvec1]] to <[[NUM]] x i32>
  // CHECK: [[and:%.*]] = and <[[NUM]] x i32> [[f2i]], <i32 2139095040
  // CHECK: [[add:%.*]] = add nsw <[[NUM]] x i32> [[and]], <i32 -1056964608
  // CHECK: [[shr:%.*]] = ashr <[[NUM]] x i32> [[add]], <i32 23
  // CHECK: [[i2f:%.*]] = sitofp <[[NUM]] x i32> [[shr]] to <[[NUM]] x float>
  // CHECK: [[sel:%.*]] = select <[[NUM]] x i1> [[cmp]], <[[NUM]] x float> [[i2f]], <[[NUM]] x float> zeroinitializer
  // CHECK: [[and:%.*]] = and <[[NUM]] x i32> [[f2i]], <i32 8388607
  // CHECK: or <[[NUM]] x i32> [[and]], <i32 1056964608
  vector<float, NUM> exp = fVec3;
  fRes += frexp(fVec1, exp);
  fRes += exp;

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = fsub fast <[[NUM]] x half> [[hvec3]], [[hvec2]]
  // CHECK: fmul fast <[[NUM]] x half> [[tmp]], [[hvec1]]
  hRes += lerp(hVec2, hVec3, hVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = fsub fast <[[NUM]] x float> [[fvec3]], [[fvec2]]
  // CHECK: fmul fast <[[NUM]] x float> [[tmp]], [[fvec1]]
  fRes += lerp(fVec2, fVec3, fVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: fdiv fast <[[NUM]] x half> <half 0xH3C00, {{.*}}>, [[hvec1]]
  hRes += rcp(hVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: fdiv fast <[[NUM]] x float> <float 1.000000e+00, {{.*}}>, [[fvec1]]
  fRes += rcp(fVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 83, <[[NUM]] x half> [[hvec1]])  ; DerivCoarseX(value)
  // CHECK: call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 6, <[[NUM]] x half> [[tmp]])  ; FAbs(value)
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 84, <[[NUM]] x half> [[hvec1]])  ; DerivCoarseY(value)
  // CHECK: call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 6, <[[NUM]] x half> [[tmp]])  ; FAbs(value)
  hRes += fwidth(hVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 83, <[[NUM]] x float> [[fvec1]])  ; DerivCoarseX(value)
  // CHECK: call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 6, <[[NUM]] x float> [[tmp]])  ; FAbs(value)
  // CHECK: [[tmp:%.*]] = call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 84, <[[NUM]] x float> [[fvec1]])  ; DerivCoarseY(value)
  // CHECK: call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 6, <[[NUM]] x float> [[tmp]])  ; FAbs(value)
  fRes += fwidth(fVec1);

  vector<uint, NUM> signs = 1;
  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[gt:%.*]] = fcmp fast ogt <[[NUM]] x half> [[hvec1]], zeroinitializer
  // CHECK: [[lt:%.*]] = fcmp fast olt <[[NUM]] x half> [[hvec1]], zeroinitializer
  // CHECK: [[igt:%.*]] = zext <[[NUM]] x i1> [[gt]] to <[[NUM]] x i32>
  // CHECK: [[ilt:%.*]] = zext <[[NUM]] x i1> [[lt]] to <[[NUM]] x i32>
  // CHECK: sub nsw <[[NUM]] x i32> [[igt]], [[ilt]]
  signs *= sign(hVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[gt:%.*]] = fcmp fast ogt <[[NUM]] x float> [[fvec1]], zeroinitializer
  // CHECK: [[lt:%.*]] = fcmp fast olt <[[NUM]] x float> [[fvec1]], zeroinitializer
  // CHECK: [[igt:%.*]] = zext <[[NUM]] x i1> [[gt]] to <[[NUM]] x i32>
  // CHECK: [[ilt:%.*]] = zext <[[NUM]] x i1> [[lt]] to <[[NUM]] x i32>
  // CHECK: sub nsw <[[NUM]] x i32> [[igt]], [[ilt]]
  signs *= sign(fVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[gt:%.*]] = fcmp fast ogt <[[NUM]] x double> [[dvec1]], zeroinitializer
  // CHECK: [[lt:%.*]] = fcmp fast olt <[[NUM]] x double> [[dvec1]], zeroinitializer
  // CHECK: [[igt:%.*]] = zext <[[NUM]] x i1> [[gt]] to <[[NUM]] x i32>
  // CHECK: [[ilt:%.*]] = zext <[[NUM]] x i1> [[lt]] to <[[NUM]] x i32>
  // CHECK: sub nsw <[[NUM]] x i32> [[igt]], [[ilt]]
  signs *= sign(dVec1);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[gt:%.*]] = icmp sgt <[[NUM]] x i16> [[svec2]], zeroinitializer
  // CHECK: [[lt:%.*]] = icmp slt <[[NUM]] x i16> [[svec2]], zeroinitializer
  // CHECK: [[igt:%.*]] = zext <[[NUM]] x i1> [[gt]] to <[[NUM]] x i32>
  // CHECK: [[ilt:%.*]] = zext <[[NUM]] x i1> [[lt]] to <[[NUM]] x i32>
  // CHECK: sub nsw <[[NUM]] x i32> [[igt]], [[ilt]]
  signs *= sign(sVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[cmp:%.*]] = icmp ne <[[NUM]] x i16> [[usvec2]], zeroinitializer
  // CHECK: zext <[[NUM]] x i1> [[cmp]] to <[[NUM]] x i32>
  signs *= sign(usVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[gt:%.*]] = icmp sgt <[[NUM]] x i32> [[ivec2]], zeroinitializer
  // CHECK: [[lt:%.*]] = icmp slt <[[NUM]] x i32> [[ivec2]], zeroinitializer
  // CHECK: [[igt:%.*]] = zext <[[NUM]] x i1> [[gt]] to <[[NUM]] x i32>
  // CHECK: [[ilt:%.*]] = zext <[[NUM]] x i1> [[lt]] to <[[NUM]] x i32>
  // CHECK: [[sub:%.*]] = sub nsw <[[NUM]] x i32> [[igt]], [[ilt]]
  signs *= sign(iVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[cmp:%.*]] = icmp ne <[[NUM]] x i32> [[uivec2]], zeroinitializer
  // CHECK: zext <[[NUM]] x i1> [[cmp]] to <[[NUM]] x i32>
  signs *= sign(uiVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[gt:%.*]] = icmp sgt <[[NUM]] x i64> [[lvec2]], zeroinitializer
  // CHECK: [[lt:%.*]] = icmp slt <[[NUM]] x i64> [[lvec2]], zeroinitializer
  // CHECK: [[igt:%.*]] = zext <[[NUM]] x i1> [[gt]] to <[[NUM]] x i32>
  // CHECK: [[ilt:%.*]] = zext <[[NUM]] x i1> [[lt]] to <[[NUM]] x i32>
  // CHECK: sub nsw <[[NUM]] x i32> [[igt]], [[ilt]]
  signs *= sign(lVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[cmp:%.*]] = icmp ne <[[NUM]] x i64> [[ulvec2]], zeroinitializer
  // CHECK: zext <[[NUM]] x i1> [[cmp]] to <[[NUM]] x i32>
  signs *= sign(ulVec2);

  iRes += signs;

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[bvec2:%.*]] = icmp ne <[[NUM]] x i16> [[svec2]], zeroinitializer
  // CHECK: [[bvec1:%.*]] = icmp ne <[[NUM]] x i16> [[svec1]], zeroinitializer
  // CHECK: or <[[NUM]] x i1> [[bvec2]], [[bvec1]]
  sRes += or(sVec1, sVec2);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: [[bvec3:%.*]] = icmp ne <[[NUM]] x i16> [[svec3]], zeroinitializer
  // CHECK: and <[[NUM]] x i1> [[bvec3]], [[bvec2]]
  sRes += and(sVec2, sVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: select <[[NUM]] x i1> [[bvec1]], <[[NUM]] x i16> [[svec2]], <[[NUM]] x i16> [[svec3]]
  sRes += select(sVec1, sVec2, sVec3);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  buf.Store<vector<float16_t, NUM> >(0, hRes);
  buf.Store<vector<float, NUM> >(2048, fRes);
  buf.Store<vector<double, NUM> >(4096, dRes);

  ibuf.Store<vector<int16_t, NUM> >(0, sRes);
  ibuf.Store<vector<uint16_t, NUM> >(1024, usRes);
  ibuf.Store<vector<int, NUM> >(2048, iRes);
  ibuf.Store<vector<uint, NUM> >(3072, uiRes);
  ibuf.Store<vector<int64_t, NUM> >(4096, lRes);
  ibuf.Store<vector<uint64_t, NUM> >(5120, ulRes);
}
