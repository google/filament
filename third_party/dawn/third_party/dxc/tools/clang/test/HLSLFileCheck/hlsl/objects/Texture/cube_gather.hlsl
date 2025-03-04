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

  ///////////////////////////////////////////////
  // Gather
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0)
  r += cube.Gather(samp, a.xyz);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.Gather(samp, a.xyz+0.05, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0)
  r += cubeArray.Gather(samp, a.xyzw);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.Gather(samp, a.xyzw+0.05, status); r += CheckAccessFullyMapped(status);

  a *= 1.125; // Prevent GatherRed from being optimized to equivalent Gather above

  ///////////////////////////////////////////////
  // GatherRed
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0)
  r += cube.GatherRed(samp, a.xyz);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.GatherRed(samp, a.xyz+0.05, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0)
  r += cubeArray.GatherRed(samp, a.xyzw);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.GatherRed(samp, a.xyzw+0.05, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // GatherGreen
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 1)
  r += cube.GatherGreen(samp, a.xyz);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 1)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.GatherGreen(samp, a.xyz+0.05, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 1)
  r += cubeArray.GatherGreen(samp, a.xyzw);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 1)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.GatherGreen(samp, a.xyzw+0.05, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // GatherBlue
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 2)
  r += cube.GatherBlue(samp, a.xyz);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 2)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.GatherBlue(samp, a.xyz+0.05, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 2)
  r += cubeArray.GatherBlue(samp, a.xyzw);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 2)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.GatherBlue(samp, a.xyzw+0.05, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // GatherAlpha
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 3)
  r += cube.GatherAlpha(samp, a.xyz);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 3)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.GatherAlpha(samp, a.xyz+0.05, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 3)
  r += cubeArray.GatherAlpha(samp, a.xyzw);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32(i32 73,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 3)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.GatherAlpha(samp, a.xyzw+0.05, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // GatherCmp
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0
  // CHECK-SAME: , float 5.000000e-01)
  r += cube.GatherCmp(sampcmp, a.xyz, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.GatherCmp(sampcmp, a.xyz+0.05, CMP, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0
  // CHECK-SAME: , float 5.000000e-01)
  r += cubeArray.GatherCmp(sampcmp, a.xyzw, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.GatherCmp(sampcmp, a.xyzw+0.05, CMP, status); r += CheckAccessFullyMapped(status);

  a *= 1.125; // Prevent GatherCmpRed from being optimized to equivalent GatherCmp above

  ///////////////////////////////////////////////
  // GatherCmpRed
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0
  // CHECK-SAME: , float 5.000000e-01)
  r += cube.GatherCmpRed(sampcmp, a.xyz, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.GatherCmpRed(sampcmp, a.xyz+0.05, CMP, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0
  // CHECK-SAME: , float 5.000000e-01)
  r += cubeArray.GatherCmpRed(sampcmp, a.xyzw, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 0
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.GatherCmpRed(sampcmp, a.xyzw+0.05, CMP, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // GatherCmpGreen
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 1
  // CHECK-SAME: , float 5.000000e-01)
  r += cube.GatherCmpGreen(sampcmp, a.xyz, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 1
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.GatherCmpGreen(sampcmp, a.xyz+0.05, CMP, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 1
  // CHECK-SAME: , float 5.000000e-01)
  r += cubeArray.GatherCmpGreen(sampcmp, a.xyzw, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 1
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.GatherCmpGreen(sampcmp, a.xyzw+0.05, CMP, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // GatherCmpBlue
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 2
  // CHECK-SAME: , float 5.000000e-01)
  r += cube.GatherCmpBlue(sampcmp, a.xyz, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 2
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.GatherCmpBlue(sampcmp, a.xyz+0.05, CMP, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 2
  // CHECK-SAME: , float 5.000000e-01)
  r += cubeArray.GatherCmpBlue(sampcmp, a.xyzw, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 2
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.GatherCmpBlue(sampcmp, a.xyzw+0.05, CMP, status); r += CheckAccessFullyMapped(status);


  ///////////////////////////////////////////////
  // GatherCmpAlpha
  ///////////////////////////////////////////////

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 3
  // CHECK-SAME: , float 5.000000e-01)
  r += cube.GatherCmpAlpha(sampcmp, a.xyz, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float undef
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 3
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cube.GatherCmpAlpha(sampcmp, a.xyz+0.05, CMP, status); r += CheckAccessFullyMapped(status);

  // TextureCubeArray
  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 3
  // CHECK-SAME: , float 5.000000e-01)
  r += cubeArray.GatherCmpAlpha(sampcmp, a.xyzw, CMP);

  // CHECK: call %dx.types.ResRet.f32 @dx.op.textureGatherCmp.f32(i32 74,
  // CHECK-SAME: , float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}, float %{{[^,]+}}
  // CHECK-SAME: , i32 undef, i32 undef
  // CHECK-SAME: , i32 3
  // CHECK-SAME: , float 5.000000e-01)
  // CHECK: extractvalue %dx.types.ResRet.f32 %{{[^,]+}}, 4
  // CHECK: call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += cubeArray.GatherCmpAlpha(sampcmp, a.xyzw+0.05, CMP, status); r += CheckAccessFullyMapped(status);


  return r;
}
