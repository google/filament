// RUN: %dxilver 1.7 | %dxc -E main -T ps_6_7 -DTYPE=float4 %s                            | FileCheck %s
// RUN: %dxilver 1.7 | %dxc -E main -T ps_6_7 -DTYPE=half4       -enable-16bit-types %s   | FileCheck %s
// RUN: %dxilver 1.7 | %dxc -E main -T ps_6_7 -DTYPE=min16float4 -enable-16bit-types %s   | FileCheck %s
// RUN: %dxilver 1.7 | %dxc -E main -T cs_6_7 -DTYPE=float4 %s                            | FileCheck %s
// RUN: %dxilver 1.7 | %dxc -E main -T cs_6_7 -DTYPE=half4       -enable-16bit-types %s   | FileCheck %s
// RUN: %dxilver 1.7 | %dxc -E main -T cs_6_7 -DTYPE=min16float4 -enable-16bit-types %s   | FileCheck %s

SamplerComparisonState samp1 : register(s5);
Texture1D<TYPE> tx1d : register(t0);
Texture1DArray<TYPE> tx1dArr : register(t1);
Texture2D<TYPE> tx2d : register(t2);
Texture2DArray<TYPE> tx2dArr : register(t3);
TextureCube<TYPE> txc : register(t4);
TextureCubeArray<TYPE> txcArr : register(t5);

RWBuffer<TYPE> buf1;

float cmpVal;

//CHECK: Advanced Texture Ops

float doit(float4 a, float lod) {
  uint status;
  float r = 0;

  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx1d.SampleCmpLevel(samp1, a.x, cmpVal, lod);
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx1d.SampleCmpLevel(samp1, a.x, cmpVal, lod, uint1(-5));
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx1d.SampleCmpLevel(samp1, a.x, cmpVal, lod, uint1(-3), status);
  r += CheckAccessFullyMapped(status);

  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx1dArr.SampleCmpLevel(samp1, a.xy, cmpVal, lod);
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx1dArr.SampleCmpLevel(samp1, a.xy, cmpVal, lod, uint1(-5));
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx1dArr.SampleCmpLevel(samp1, a.xy, cmpVal, lod, uint1(-3), status);
  r += CheckAccessFullyMapped(status);

  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx2d.SampleCmpLevel(samp1, a.xy, cmpVal, lod);
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx2d.SampleCmpLevel(samp1, a.xy, cmpVal, lod, uint2(-5, 7));
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx2d.SampleCmpLevel(samp1, a.xy, cmpVal, lod, uint2(-3, 2), status);
  r += CheckAccessFullyMapped(status);

  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx2dArr.SampleCmpLevel(samp1, a.xyz, cmpVal, lod);
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx2dArr.SampleCmpLevel(samp1, a.xyz, cmpVal, lod, uint2(-5, 7));
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += tx2dArr.SampleCmpLevel(samp1, a.xyz, cmpVal, lod, uint2(-3, 2), status);
  r += CheckAccessFullyMapped(status);

  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += txc.SampleCmpLevel(samp1, a.xyz, cmpVal, lod);
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += txc.SampleCmpLevel(samp1, a.zyx, cmpVal, lod, status);
  r += CheckAccessFullyMapped(status);

  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += txcArr.SampleCmpLevel(samp1, a, cmpVal, lod);
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevel.f32(i32 224,
  r += txcArr.SampleCmpLevel(samp1, a.wzyx, cmpVal, lod, status);
  r += CheckAccessFullyMapped(status);

  return r;
}

#if __SHADER_TARGET_STAGE == __SHADER_STAGE_COMPUTE
[numthreads(8, 8, 8)]
void main(uint dtID : SV_DispatchThreadID) {
  float4 a = buf1[dtID];
  float lod = buf1[dtID].z;
  buf1[dtID] = doit(a, lod);
}
#else
float main(float4 a : A, float lod : L) : SV_Target {
  return doit(a, lod);
}
#endif
