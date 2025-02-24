// RUN: %dxc -E main -T ps_6_0 %s   | FileCheck %s

SamplerState samp : register(s0);
SamplerComparisonState sampcmp : register(s1);
TextureCube<float4> cube : register(t0);
TextureCubeArray<float4> cubeArray : register(t1);

#define LOD 11
#define CMP 0.5

float4 main(float4 a : A) : SV_Target
{
  uint status;
  float4 r = 0;
  const float bias = 0.25;
  float4 dx = ddx_fine(a);
  float4 dy = ddy_fine(a);

  ///////////////////////////////////////////////
  // Sample
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float undef)
  r += cube.Sample(samp, a.xyz);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 1.100000e+01)
  r += cube.Sample(samp, a.xyz, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.Sample(samp, a.xyz, LOD+1, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float undef)
  r += cubeArray.Sample(samp, a.xyzw);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 1.100000e+01)
  r += cubeArray.Sample(samp, a.xyzw, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.Sample(samp, a.xyzw, LOD+1, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // SampleLevel
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 1.100000e+01)
  r += cube.SampleLevel(samp, a.xyz, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.SampleLevel(samp, a.xyz, LOD+1, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
  // CHECK: , float 1.100000e+01)
  r += cubeArray.SampleLevel(samp, a.xyzw, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.SampleLevel(samp, a.xyzw, LOD+1, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // SampleBias
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 2.500000e-01, float undef)
  r += cube.SampleBias(samp, a.xyz, bias);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 2.500000e-01, float 1.100000e+01)
  r += cube.SampleBias(samp, a.xyz, bias, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 2.500000e-01, float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.SampleBias(samp, a.xyz, bias, LOD+1, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 2.500000e-01, float undef)
  r += cubeArray.SampleBias(samp, a.xyzw, bias);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 2.500000e-01, float 1.100000e+01)
  r += cubeArray.SampleBias(samp, a.xyzw, bias, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 2.500000e-01, float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.SampleBias(samp, a.xyzw, bias, LOD+1, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // SampleGrad
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float undef)
  r += cube.SampleGrad(samp, a.xyz, dx.xyz, dy.xyz);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float 1.100000e+01)
  r += cube.SampleGrad(samp, a.xyz, dx.xyz, dy.xyz, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.SampleGrad(samp, a.xyz, dx.xyz, dy.xyz, LOD+1, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float undef)
  r += cubeArray.SampleGrad(samp, a.xyzw, dx.xyz, dy.xyz);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float 1.100000e+01)
  r += cubeArray.SampleGrad(samp, a.xyzw, dx.xyz, dy.xyz, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.SampleGrad(samp, a.xyzw, dx.xyz, dy.xyz, LOD+1, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // SampleCmp
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 5.000000e-01, float undef)
  r += cube.SampleCmp(sampcmp, a.xyz, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 5.000000e-01, float 1.100000e+01)
  r += cube.SampleCmp(sampcmp, a.xyz, CMP, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 5.000000e-01, float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.SampleCmp(sampcmp, a.xyz, CMP, LOD+1, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 5.000000e-01, float undef)
  r += cubeArray.SampleCmp(sampcmp, a.xyzw, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 5.000000e-01, float 1.100000e+01)
  r += cubeArray.SampleCmp(sampcmp, a.xyzw, CMP, LOD);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 5.000000e-01, float 1.200000e+01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.SampleCmp(sampcmp, a.xyzw, CMP, LOD+1, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // SampleCmpLevelZero
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 5.000000e-01)
  r += cube.SampleCmpLevelZero(sampcmp, a.xyz, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 1.500000e+00)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.SampleCmpLevelZero(sampcmp, a.xyz, CMP+1, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 5.000000e-01)
  r += cubeArray.SampleCmpLevelZero(sampcmp, a.xyzw, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65,
  // CHECK: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK: , i32 undef, i32 undef, i32 undef
  // CHECK: , float 1.500000e+00)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.SampleCmpLevelZero(sampcmp, a.xyzw, CMP+1, status); r += CheckAccessFullyMapped(status);


  return r;
}
