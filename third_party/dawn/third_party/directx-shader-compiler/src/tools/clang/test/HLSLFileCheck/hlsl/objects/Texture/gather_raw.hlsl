// RUN: %dxc -T ps_6_7 -enable-16bit-types %s  | FileCheck %s

//CHECK: Advanced Texture Ops


// CHECK: call %dx.types.ResRet.i16 @dx.op.textureGatherRaw.i16
// CHECK: call %dx.types.ResRet.i16 @dx.op.textureGatherRaw.i16
// CHECK: call %dx.types.ResRet.i16 @dx.op.textureGatherRaw.i16
// CHECK: call %dx.types.ResRet.i16 @dx.op.textureGatherRaw.i16
// CHECK: call %dx.types.ResRet.i16 @dx.op.textureGatherRaw.i16
// CHECK: call %dx.types.ResRet.i16 @dx.op.textureGatherRaw.i16

// CHECK: call %dx.types.ResRet.i32 @dx.op.textureGatherRaw.i32
// CHECK: call %dx.types.ResRet.i32 @dx.op.textureGatherRaw.i32
// CHECK: call %dx.types.ResRet.i32 @dx.op.textureGatherRaw.i32
// CHECK: call %dx.types.ResRet.i32 @dx.op.textureGatherRaw.i32
// CHECK: call %dx.types.ResRet.i32 @dx.op.textureGatherRaw.i32
// CHECK: call %dx.types.ResRet.i32 @dx.op.textureGatherRaw.i32

// CHECK: call %dx.types.ResRet.i64 @dx.op.textureGatherRaw.i64
// CHECK: call %dx.types.ResRet.i64 @dx.op.textureGatherRaw.i64
// CHECK: call %dx.types.ResRet.i64 @dx.op.textureGatherRaw.i64
// CHECK: call %dx.types.ResRet.i64 @dx.op.textureGatherRaw.i64
// CHECK: call %dx.types.ResRet.i64 @dx.op.textureGatherRaw.i64
// CHECK: call %dx.types.ResRet.i64 @dx.op.textureGatherRaw.i64

Texture2D<uint16_t> g_tex16 : register(t1);
Texture2D<uint32_t> g_tex32 : register(t2);
Texture2D<uint64_t> g_tex64 : register(t3);

Texture2DArray<uint16_t> g_texArr16 : register(t4);
Texture2DArray<uint32_t> g_texArr32 : register(t5);
Texture2DArray<uint64_t> g_texArr64 : register(t6);

SamplerState g_samp : register(s5);

float4 main(float2 a : A) : SV_Target
{
  uint status;
  uint16_t4 r16 = 0;
  float3 b = float3(a.x, a.y, 1.0);
  r16 += g_tex16.GatherRaw(g_samp, a);
  r16 += g_tex16.GatherRaw(g_samp, a, uint2(-5, 7));
  r16 += g_tex16.GatherRaw(g_samp, a, uint2(-3, 2), status); r16 += !!status;

  r16 += g_texArr16.GatherRaw(g_samp, b);
  r16 += g_texArr16.GatherRaw(g_samp, b, uint2(-5, 7));
  r16 += g_texArr16.GatherRaw(g_samp, b, uint2(-3, 2), status); r16 += !!status;

  uint32_t4 r32 = 0;
  r32 += g_tex32.GatherRaw(g_samp, a);
  r32 += g_tex32.GatherRaw(g_samp, a, uint2(-5, 7));
  r32 += g_tex32.GatherRaw(g_samp, a, uint2(-3, 2), status); r32 += !!status;

  r32 += g_texArr32.GatherRaw(g_samp, b);
  r32 += g_texArr32.GatherRaw(g_samp, b, uint2(-5, 7));
  r32 += g_texArr32.GatherRaw(g_samp, b, uint2(-3, 2), status); r32 += !!status;

  uint64_t4 r64 = 0;
  r64 += g_tex64.GatherRaw(g_samp, a);
  r64 += g_tex64.GatherRaw(g_samp, a, uint2(-5, 7));
  r64 += g_tex64.GatherRaw(g_samp, a, uint2(-3, 2), status); r64 += !!status;

  r64 += g_texArr64.GatherRaw(g_samp, b);
  r64 += g_texArr64.GatherRaw(g_samp, b, uint2(-5, 7));
  r64 += g_texArr64.GatherRaw(g_samp, b, uint2(-3, 2), status); r64 += !!status;

  return r16 + r32 + (r64 & 0xFFFFFFFF) + (r64 >> 32);
}

