// RUN: %dxc -DMIDX=1 -DVIDX=2 -T ps_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=2 -T ps_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=1 -DVIDX=j -T ps_6_0 %s | FileCheck %s
// RUN: %dxc -DMIDX=i -DVIDX=j -T ps_6_0 %s | FileCheck %s

// RUN: %dxc -DMIDX=1 -DVIDX=2 -T cs_6_0 -E CSMain -DGS %s | FileCheck %s -check-prefix=CSCHK
// RUN: %dxc -DMIDX=i -DVIDX=2 -T cs_6_0 -E CSMain -DGS %s | FileCheck %s -check-prefix=CSCHK
// RUN: %dxc -DMIDX=1 -DVIDX=j -T cs_6_0 -E CSMain -DGS %s | FileCheck %s -check-prefix=CSCHK
// RUN: %dxc -DMIDX=i -DVIDX=j -T cs_6_0 -E CSMain -DGS %s | FileCheck %s -check-prefix=CSCHK
// RUN: %dxc -DMIDX=1 -DVIDX=2 -T lib_6_3 %s -DGS | FileCheck %s -check-prefixes=CSCHK,CHECK
// RUN: %dxc -DMIDX=i -DVIDX=2 -T lib_6_3 %s -DGS | FileCheck %s -check-prefixes=CSCHK,CHECK
// RUN: %dxc -DMIDX=1 -DVIDX=j -T lib_6_3 %s -DGS | FileCheck %s -check-prefixes=CSCHK,CHECK
// RUN: %dxc -DMIDX=i -DVIDX=j -T lib_6_3 %s -DGS | FileCheck %s -check-prefixes=CSCHK,CHECK

// Test for general subscript operations on matrix arrays.
// Specifically focused on shader inputs which failed to lower previously

float3 GetRow(const float3x3 m, const int j)
{
  return m[j];
}

float3x3 g[2];
groupshared float3x3 gs[2];

struct JustMtx {
  float3x3 mtx;
};

struct MtxArray {
  float3x3 mtx[2];
};

RWStructuredBuffer<float3> output;

[shader("compute")]
[numthreads(8,8,1)]
void CSMain(uint3 gtid : SV_GroupThreadID, uint ix : SV_GroupIndex)
{
  float3 ret = 0.0;
  uint i = gtid.x;
  uint j = gtid.y;

  // CSCHK: load float, float addrspace(3)*
  // CSCHK: load float, float addrspace(3)*
  // CSCHK: load float, float addrspace(3)*
  ret += gs[MIDX][VIDX];

  ret += GetRow(gs[MIDX], VIDX);

  output[ix] = ret;
}

[shader("pixel")]
float3 main(const int i : I, const int j : J, const float3x3 m[2]: M, JustMtx jm[2] : JM, MtxArray ma : A) : SV_Target
{
  float3 ret = 0.0;

  // CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32
  // CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32
  // CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32
  ret += g[MIDX][VIDX];

  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 2, i32 {{%?[0-9]*}}, i8 2, i32 undef)
  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 2, i32 {{%?[0-9]*}}, i8 2, i32 undef)
  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 2, i32 {{%?[0-9]*}}, i8 2, i32 undef)
  ret += m[MIDX][VIDX];

  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 3, i32 {{%?[0-9]*}}, i8 2, i32 undef)
  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 3, i32 {{%?[0-9]*}}, i8 2, i32 undef)
  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 3, i32 {{%?[0-9]*}}, i8 2, i32 undef)
  ret += jm[MIDX].mtx[VIDX];

  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 4, i32 {{%?[0-9]*}}, i8 2, i32 undef)
  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 4, i32 {{%?[0-9]*}}, i8 2, i32 undef)
  // CHECK: call float @dx.op.loadInput.f32(i32 4, i32 4, i32 {{%?[0-9]*}}, i8 2, i32 undef)
  ret += ma.mtx[MIDX][VIDX];

  ret += GetRow(g[MIDX], VIDX);
  ret += GetRow(m[MIDX], VIDX);
  ret += GetRow(jm[MIDX].mtx, VIDX);
  ret += GetRow(ma.mtx[MIDX], VIDX);

  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %{{.*}})
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %{{.*}})
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %{{.*}})
  return ret;
}

