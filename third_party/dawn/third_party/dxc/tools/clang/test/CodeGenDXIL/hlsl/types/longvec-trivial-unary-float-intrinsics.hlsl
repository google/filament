// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=abs  -DOP=6 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=abs  -DOP=6 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=saturate  -DOP=7 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=saturate  -DOP=7 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=cos  -DOP=12 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=cos  -DOP=12 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=sin  -DOP=13 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=sin  -DOP=13 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=tan  -DOP=14 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=tan  -DOP=14 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=acos -DOP=15 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=acos -DOP=15 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=asin -DOP=16 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=asin -DOP=16 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=atan -DOP=17 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=atan -DOP=17 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=cosh -DOP=18 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=cosh -DOP=18 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=sinh -DOP=19 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=sinh -DOP=19 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=tanh -DOP=20 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=tanh -DOP=20 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=exp2 -DOP=21 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=exp2 -DOP=21 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=frac -DOP=22 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=frac -DOP=22 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=log2 -DOP=23 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=log2 -DOP=23 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=log10 -DOP=23 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=log10 -DOP=23 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=sqrt -DOP=24 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=sqrt -DOP=24 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=rsqrt -DOP=25 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=rsqrt -DOP=25 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=round -DOP=26 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=round -DOP=26 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=floor -DOP=27 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=floor -DOP=27 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ceil -DOP=28 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ceil -DOP=28 -DNUM=1022 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=trunc -DOP=29 -DNUM=7    %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=trunc -DOP=29 -DNUM=1022 %s | FileCheck %s

// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddx -DOP=83 -DNUM=7    %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddx -DOP=83 -DNUM=1022 %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddx_coarse -DOP=83 -DNUM=7    %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddx_coarse -DOP=83 -DNUM=1022 %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddx_fine -DOP=85 -DNUM=7    %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddx_fine -DOP=85 -DNUM=1022 %s | FileCheck %s -check-prefixes=CHECK,CONV

// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddy -DOP=84 -DNUM=7    %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddy -DOP=84 -DNUM=1022 %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddy_coarse -DOP=84 -DNUM=7    %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddy_coarse -DOP=84 -DNUM=1022 %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddy_fine -DOP=86 -DNUM=7    %s | FileCheck %s -check-prefixes=CHECK,CONV
// RUN: %dxc -T cs_6_9 -enable-16bit-types -DFUNC=ddy_fine -DOP=86 -DNUM=1022 %s | FileCheck %s -check-prefixes=CHECK,CONV

// Test vector-enabled unary intrinsics that take float-like parameters and
// and are "trivial" in that they can be implemented with a single call
// instruction with the same parameter and return types.

RWByteAddressBuffer buf;

// CHECK-DAG: %dx.types.ResRet.[[HTY:v[0-9]*f16]] = type { <[[NUM:[0-9]*]] x half>
// CHECK-DAG: %dx.types.ResRet.[[FTY:v[0-9]*f32]] = type { <[[NUM]] x float>

[numthreads(8,1,1)]
void main() {

  // Capture opcode number.
  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })
  // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[buf]], i32 999, i32 undef, i32 [[OP:[0-9]*]]
  buf.Store(999, OP);

  // CHECK: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[HTY]] @dx.op.rawBufferVectorLoad.[[HTY]](i32 303, %dx.types.Handle [[buf]], i32 0
  // CHECK: [[hvec:%.*]] = extractvalue %dx.types.ResRet.[[HTY]] [[ld]], 0
  vector<float16_t, NUM> hVec = buf.Load<vector<float16_t, NUM> >(0);

  // Convergent markers prevent GVN removal of redundant annotateHandle calls.
  // CONV: [[buf:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })

  // CHECK: [[ld:%.*]] = call %dx.types.ResRet.[[FTY]] @dx.op.rawBufferVectorLoad.[[FTY]](i32 303, %dx.types.Handle [[buf]], i32 1024
  // CHECK: [[fvec:%.*]] = extractvalue %dx.types.ResRet.[[FTY]] [[ld]], 0
  vector<float, NUM> fVec = buf.Load<vector<float, NUM> >(1024);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x half> @dx.op.unary.[[HTY]](i32 [[OP]], <[[NUM]] x half> [[hvec]])
  vector<float16_t, NUM> hRes = FUNC(hVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  // CHECK: call <[[NUM]] x float> @dx.op.unary.[[FTY]](i32 [[OP]], <[[NUM]] x float> [[fvec]])
  vector<float, NUM> fRes = FUNC(fVec);

  // CHECK-NOT: extractelement
  // CHECK-NOT: insertelement
  buf.Store<vector<float16_t, NUM> >(0, hRes);
  buf.Store<vector<float, NUM> >(1024, fRes);
}
