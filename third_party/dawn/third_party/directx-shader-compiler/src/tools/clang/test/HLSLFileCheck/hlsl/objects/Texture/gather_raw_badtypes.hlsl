// RUN: %dxc -T ps_6_7 -enable-16bit-types %s  | FileCheck %s

Texture2D<float16_t> g_texf16 : register(t1);
Texture2D<float32_t> g_texf32 : register(t2);
Texture2D<float64_t> g_texf64 : register(t3);

Texture2DArray<float16_t> g_texArrf16 : register(t4);
Texture2DArray<float32_t> g_texArrf32 : register(t5);
Texture2DArray<float64_t> g_texArrf64 : register(t6);

Texture2D<int16_t4> g_tex16 : register(t7);
Texture2D<int32_t3> g_tex32 : register(t8);
Texture2D<int64_t2> g_tex64 : register(t9);

Texture2DArray<uint16_t4> g_texArr16 : register(t10);
Texture2DArray<uint32_t3> g_texArr32 : register(t11);
Texture2DArray<uint64_t2> g_texArr64 : register(t12);

SamplerState g_samp : register(s5);

//CHECK:  error: cannot GatherRaw from resource containing half
//CHECK:  error: cannot GatherRaw from resource containing half
//CHECK:  error: cannot GatherRaw from resource containing half
//CHECK:  error: cannot GatherRaw from resource containing half
//CHECK:  error: cannot GatherRaw from resource containing half
//CHECK:  error: cannot GatherRaw from resource containing half

//CHECK:  error: cannot GatherRaw from resource containing float
//CHECK:  error: cannot GatherRaw from resource containing float
//CHECK:  error: cannot GatherRaw from resource containing float
//CHECK:  error: cannot GatherRaw from resource containing float
//CHECK:  error: cannot GatherRaw from resource containing float
//CHECK:  error: cannot GatherRaw from resource containing float

//CHECK:  error: cannot GatherRaw from resource containing double
//CHECK:  error: cannot GatherRaw from resource containing double
//CHECK:  error: cannot GatherRaw from resource containing double
//CHECK:  error: cannot GatherRaw from resource containing double
//CHECK:  error: cannot GatherRaw from resource containing double
//CHECK:  error: cannot GatherRaw from resource containing double

//CHECK: error: cannot GatherRaw from resource containing short4
//CHECK: error: cannot GatherRaw from resource containing short4
//CHECK: error: cannot GatherRaw from resource containing short4

//CHECK: error: cannot GatherRaw from resource containing ushort4
//CHECK: error: cannot GatherRaw from resource containing ushort4
//CHECK: error: cannot GatherRaw from resource containing ushort4

//CHECK: error: cannot GatherRaw from resource containing int3
//CHECK: error: cannot GatherRaw from resource containing int3
//CHECK: error: cannot GatherRaw from resource containing int3

//CHECK: error: cannot GatherRaw from resource containing uint3
//CHECK: error: cannot GatherRaw from resource containing uint3
//CHECK: error: cannot GatherRaw from resource containing uint3

//CHECK: error: cannot GatherRaw from resource containing long2
//CHECK: error: cannot GatherRaw from resource containing long2
//CHECK: error: cannot GatherRaw from resource containing long2

//CHECK: error: cannot GatherRaw from resource containing ulong2
//CHECK: error: cannot GatherRaw from resource containing ulong2
//CHECK: error: cannot GatherRaw from resource containing ulong2

float4 main(float2 a : A) : SV_Target
{
  uint status;
  uint16_t4 r16 = 0;
  float3 b = float3(a.x, a.y, 1.0);
  r16 += g_texf16.GatherRaw(g_samp, a);
  r16 += g_texf16.GatherRaw(g_samp, a, uint2(-5, 7));
  r16 += g_texf16.GatherRaw(g_samp, a, uint2(-3, 2), status); r16 += !!status;

  r16 += g_texArrf16.GatherRaw(g_samp, b);
  r16 += g_texArrf16.GatherRaw(g_samp, b, uint2(-5, 7));
  r16 += g_texArrf16.GatherRaw(g_samp, b, uint2(-3, 2), status); r16 += !!status;

  uint32_t4 r32 = 0;
  r32 += g_texf32.GatherRaw(g_samp, a);
  r32 += g_texf32.GatherRaw(g_samp, a, uint2(-5, 7));
  r32 += g_texf32.GatherRaw(g_samp, a, uint2(-3, 2), status); r32 += !!status;

  r32 += g_texArrf32.GatherRaw(g_samp, b);
  r32 += g_texArrf32.GatherRaw(g_samp, b, uint2(-5, 7));
  r32 += g_texArrf32.GatherRaw(g_samp, b, uint2(-3, 2), status); r32 += !!status;

  uint64_t4 r64 = 0;
  r64 += g_texf64.GatherRaw(g_samp, a);
  r64 += g_texf64.GatherRaw(g_samp, a, uint2(-5, 7));
  r64 += g_texf64.GatherRaw(g_samp, a, uint2(-3, 2), status); r64 += !!status;

  r64 += g_texArrf64.GatherRaw(g_samp, b);
  r64 += g_texArrf64.GatherRaw(g_samp, b, uint2(-5, 7));
  r64 += g_texArrf64.GatherRaw(g_samp, b, uint2(-3, 2), status); r64 += !!status;

  r16 += g_tex16.GatherRaw(g_samp, a);
  r16 += g_tex16.GatherRaw(g_samp, a, uint2(-5, 7));
  r16 += g_tex16.GatherRaw(g_samp, a, uint2(-3, 2), status); r16 += !!status;

  r16 += g_texArr16.GatherRaw(g_samp, b);
  r16 += g_texArr16.GatherRaw(g_samp, b, uint2(-5, 7));
  r16 += g_texArr16.GatherRaw(g_samp, b, uint2(-3, 2), status); r16 += !!status;

  r32 += g_tex32.GatherRaw(g_samp, a);
  r32 += g_tex32.GatherRaw(g_samp, a, uint2(-5, 7));
  r32 += g_tex32.GatherRaw(g_samp, a, uint2(-3, 2), status); r32 += !!status;

  r32 += g_texArr32.GatherRaw(g_samp, b);
  r32 += g_texArr32.GatherRaw(g_samp, b, uint2(-5, 7));
  r32 += g_texArr32.GatherRaw(g_samp, b, uint2(-3, 2), status); r32 += !!status;

  r64 += g_tex64.GatherRaw(g_samp, a);
  r64 += g_tex64.GatherRaw(g_samp, a, uint2(-5, 7));
  r64 += g_tex64.GatherRaw(g_samp, a, uint2(-3, 2), status); r64 += !!status;

  r64 += g_texArr64.GatherRaw(g_samp, b);
  r64 += g_texArr64.GatherRaw(g_samp, b, uint2(-5, 7));
  r64 += g_texArr64.GatherRaw(g_samp, b, uint2(-3, 2), status); r64 += !!status;

  return r16 + r32 + (r64 & 0xFFFFFFFF) + (r64 >> 32);
}
