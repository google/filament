// RUN: %dxc -DSTAGE=1 -T cs_6_6 %s | FileCheck %s
// RUN: %dxc -DSTAGE=2 -T as_6_6 %s | FileCheck %s -check-prefixes=CHECK,ASMSCHECK
// RUN: %dxc -DSTAGE=3 -T ms_6_6 %s | FileCheck %s -check-prefixes=CHECK,ASMSCHECK
// RUN: %dxilver 1.6 | %dxc -DSTAGE=1 -T cs_6_5 -Wno-hlsl-availability %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DSTAGE=2 -T as_6_5 -Wno-hlsl-availability %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DSTAGE=3 -T ms_6_5 -Wno-hlsl-availability %s | FileCheck %s -check-prefix=ERRCHECK

#define CS 1
#define AS 2
#define MS 3

// Test 6.6 feature allowing derivative operations in compute shaders

// ASMSCHECK: Note: shader requires additional functionality:
// ASMSCHECK: Derivatives in mesh and amplification shaders

Texture2D<float4> input: register( t2 );
RWTexture2D<float4> output: register( u2 );
SamplerState samp : register(s5);
SamplerComparisonState cmpSamp : register(s6);
float cmpVal;

[numthreads( 8, 8, 1 )]
#if STAGE==MS
[outputtopology("triangle")]
#endif
void main( uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID )
{
  float2 uv = DTid.xy/float2(8, 8);
  float4 res = 0;
  uint status = 0;

  // CHECK: call float @dx.op.unary.f32(i32 83,
  // CHECK: call float @dx.op.unary.f32(i32 83,
  // CHECK: call float @dx.op.unary.f32(i32 83,
  // CHECK: call float @dx.op.unary.f32(i32 83,
  // CHECK: call float @dx.op.unary.f32(i32 85,
  // CHECK: call float @dx.op.unary.f32(i32 85,
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  res.xy = ddx(uv);
  res.zw = ddx_coarse(res.xy);
  res.xz = ddx_fine(res.zw);

  // CHECK: call float @dx.op.unary.f32(i32 84,
  // CHECK: call float @dx.op.unary.f32(i32 84,
  // CHECK: call float @dx.op.unary.f32(i32 84,
  // CHECK: call float @dx.op.unary.f32(i32 84,
  // CHECK: call float @dx.op.unary.f32(i32 86,
  // CHECK: call float @dx.op.unary.f32(i32 86,
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  res.yw = ddy(res.xz);
  res.xw = ddy_coarse(res.yw);
  res.yz = ddy_fine(res.xw);

  // CHECK: @dx.op.calculateLOD.f32(i32 81
  // CHECK: @dx.op.calculateLOD.f32(i32 81
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  res += input.CalculateLevelOfDetail(samp, uv);
  res += input.CalculateLevelOfDetailUnclamped(samp, uv);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  res += input.Sample(samp, uv);
  res += input.Sample(samp, uv, uint2(-1, 1));
  res += input.Sample(samp, uv, uint2(-1, 1), .5);
  res += input.Sample(samp, uv, uint2(-1, 1), .6, status);
  res -= status;

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  res += input.SampleBias(samp, uv, 1.0);
  res += input.SampleBias(samp, uv, 1.0, uint2(-2, 2));
  res += input.SampleBias(samp, uv, 1.0, uint2(-2, 2), .7);
  res += input.SampleBias(samp, uv, 1.0, uint2(-2, 2), .8, status);
  res /= status;

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode 'Derivatives in CS/MS/AS' should only be used in 'Shader Model 6.6+'
  res += input.SampleCmp(cmpSamp, uv, cmpVal);
  res += input.SampleCmp(cmpSamp, uv, cmpVal, uint2(-3, 4));
  res += input.SampleCmp(cmpSamp, uv, cmpVal, uint2(-4, 6), DTid.z);
  res += input.SampleCmp(cmpSamp, uv, cmpVal, uint2(-5, 7), DTid.z, status);
  res *= status;

#if STAGE == AS
  struct {float4 f;} pld;
  pld.f = res;
  DispatchMesh(8, 8, 1, pld);
#else
  output[uv] = res;
#endif
}
