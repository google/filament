// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 %s | FileCheck %s
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 4, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 12, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 24, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 40, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 44, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 52, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 64, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 80, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 84, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 92, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 104, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 120, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 124, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 132, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 144, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 160, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 168, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 184, i8 15, i32 8)
// second half of double3
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 200, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 208, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 224, i8 15, i32 8)

// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 4, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 12, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 24, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 40, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 44, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 52, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 64, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 80, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 84, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 92, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 104, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 120, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 124, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 132, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 144, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 160, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 168, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 184, i8 15, i32 8)
// second half of double3
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 200, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 208, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 224, i8 15, i32 8)

struct MyStruct {
  int   i1;
  int2  i2;
  int3  i3;
  int4  i4;
  uint  u1;
  uint2 u2;
  uint3 u3;
  uint4 u4;
  half  h1;
  half2 h2;
  half3 h3;
  half4 h4;
  float f1;
  float2 f2;
  float3 f3;
  float4 f4;
  double d1;
  double2 d2;
  double3 d3;
  double4 d4;
};
StructuredBuffer<MyStruct> buf1;
RWStructuredBuffer<MyStruct> buf2;

// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_i1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_i2_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_i3_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_i4_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_u1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_u2_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_u3_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_u4_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_h1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_h2_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_h3_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_h4_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_f1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_f2_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_f3_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_f4_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_d1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_d2_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_d3_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 8)
// second half of double3
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_d3_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 16, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_d4_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_d4_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 16, i8 15, i32 8)

StructuredBuffer<int> buf1_i1;
StructuredBuffer<int2> buf1_i2;
StructuredBuffer<int3> buf1_i3;
StructuredBuffer<int4> buf1_i4;
StructuredBuffer<uint> buf1_u1;
StructuredBuffer<uint2> buf1_u2;
StructuredBuffer<uint3> buf1_u3;
StructuredBuffer<uint4> buf1_u4;
StructuredBuffer<half> buf1_h1;
StructuredBuffer<half2> buf1_h2;
StructuredBuffer<half3> buf1_h3;
StructuredBuffer<half4> buf1_h4;
StructuredBuffer<float> buf1_f1;
StructuredBuffer<float2> buf1_f2;
StructuredBuffer<float3> buf1_f3;
StructuredBuffer<float4> buf1_f4;
StructuredBuffer<double> buf1_d1;
StructuredBuffer<double2> buf1_d2;
StructuredBuffer<double3> buf1_d3;
StructuredBuffer<double4> buf1_d4;

// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_i1_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_i2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_i3_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_i4_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_u1_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_u2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_u3_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_u4_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_h1_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_h2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_h3_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_h4_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_f1_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_f2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_f3_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_f4_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_d1_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_d2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_d3_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 8)
// second half of double3
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_d3_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 16, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_d4_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_d4_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 16, i8 15, i32 8)

RWStructuredBuffer<int> buf2_i1;
RWStructuredBuffer<int2> buf2_i2;
RWStructuredBuffer<int3> buf2_i3;
RWStructuredBuffer<int4> buf2_i4;
RWStructuredBuffer<uint> buf2_u1;
RWStructuredBuffer<uint2> buf2_u2;
RWStructuredBuffer<uint3> buf2_u3;
RWStructuredBuffer<uint4> buf2_u4;
RWStructuredBuffer<half> buf2_h1;
RWStructuredBuffer<half2> buf2_h2;
RWStructuredBuffer<half3> buf2_h3;
RWStructuredBuffer<half4> buf2_h4;
RWStructuredBuffer<float> buf2_f1;
RWStructuredBuffer<float2> buf2_f2;
RWStructuredBuffer<float3> buf2_f3;
RWStructuredBuffer<float4> buf2_f4;
RWStructuredBuffer<double> buf2_d1;
RWStructuredBuffer<double2> buf2_d2;
RWStructuredBuffer<double3> buf2_d3;
RWStructuredBuffer<double4> buf2_d4;


int4 main(float idx1 : IDX1, float idx2 : IDX2) : SV_Target {
  uint status;
  float4 r = 0;
  r.x += buf1.Load(idx1, status).i1;
  r.xy += buf1.Load(idx1, status).i2;
  r.xyz += buf1.Load(idx1, status).i3;
  r.xyzw += buf1.Load(idx1, status).i4;
  r.x += buf1.Load(idx1, status).u1;
  r.xy += buf1.Load(idx1, status).u2;
  r.xyz += buf1.Load(idx1, status).u3;
  r.xyzw += buf1.Load(idx1, status).u4;
  r.x += buf1.Load(idx1, status).h1;
  r.xy += buf1.Load(idx1, status).h2;
  r.xyz += buf1.Load(idx1, status).h3;
  r.xyzw += buf1.Load(idx1, status).h4;
  r.x += buf1.Load(idx1, status).f1;
  r.xy += buf1.Load(idx1, status).f2;
  r.xyz += buf1.Load(idx1, status).f3;
  r.xyzw += buf1.Load(idx1, status).f4;
  r.x += buf1.Load(idx1, status).d1;
  r.xy += buf1.Load(idx1, status).d2;
  r.xyz += buf1.Load(idx1, status).d3;
  r.xyzw += buf1.Load(idx1, status).d4;

  r.x += buf2.Load(idx2, status).i1;
  r.xy += buf2.Load(idx2, status).i2;
  r.xyz += buf2.Load(idx2, status).i3;
  r.xyzw += buf2.Load(idx2, status).i4;
  r.x += buf2.Load(idx2, status).u1;
  r.xy += buf2.Load(idx2, status).u2;
  r.xyz += buf2.Load(idx2, status).u3;
  r.xyzw += buf2.Load(idx2, status).u4;
  r.x += buf2.Load(idx2, status).h1;
  r.xy += buf2.Load(idx2, status).h2;
  r.xyz += buf2.Load(idx2, status).h3;
  r.xyzw += buf2.Load(idx2, status).h4;
  r.x += buf2.Load(idx2, status).f1;
  r.xy += buf2.Load(idx2, status).f2;
  r.xyz += buf2.Load(idx2, status).f3;
  r.xyzw += buf2.Load(idx2, status).f4;
  r.x += buf2.Load(idx2, status).d1;
  r.xy += buf2.Load(idx2, status).d2;
  r.xyz += buf2.Load(idx2, status).d3;
  r.xyzw += buf2.Load(idx2, status).d4;

  // Basic types
  r.x += buf1_i1.Load(idx1, status);
  r.xy += buf1_i2.Load(idx1, status);
  r.xyz += buf1_i3.Load(idx1, status);
  r.xyzw += buf1_i4.Load(idx1, status);
  r.x += buf1_u1.Load(idx1, status);
  r.xy += buf1_u2.Load(idx1, status);
  r.xyz += buf1_u3.Load(idx1, status);
  r.xyzw += buf1_u4.Load(idx1, status);
  r.x += buf1_h1.Load(idx1, status);
  r.xy += buf1_h2.Load(idx1, status);
  r.xyz += buf1_h3.Load(idx1, status);
  r.xyzw += buf1_h4.Load(idx1, status);
  r.x += buf1_f1.Load(idx1, status);
  r.xy += buf1_f2.Load(idx1, status);
  r.xyz += buf1_f3.Load(idx1, status);
  r.xyzw += buf1_f4.Load(idx1, status);
  r.x += buf1_d1.Load(idx1, status);
  r.xy += buf1_d2.Load(idx1, status);
  r.xyz += buf1_d3.Load(idx1, status);
  r.xyzw += buf1_d4.Load(idx1, status);

  r.x += buf2_i1.Load(idx2, status);
  r.xy += buf2_i2.Load(idx2, status);
  r.xyz += buf2_i3.Load(idx2, status);
  r.xyzw += buf2_i4.Load(idx2, status);
  r.x += buf2_u1.Load(idx2, status);
  r.xy += buf2_u2.Load(idx2, status);
  r.xyz += buf2_u3.Load(idx2, status);
  r.xyzw += buf2_u4.Load(idx2, status);
  r.x += buf2_h1.Load(idx2, status);
  r.xy += buf2_h2.Load(idx2, status);
  r.xyz += buf2_h3.Load(idx2, status);
  r.xyzw += buf2_h4.Load(idx2, status);
  r.x += buf2_f1.Load(idx2, status);
  r.xy += buf2_f2.Load(idx2, status);
  r.xyz += buf2_f3.Load(idx2, status);
  r.xyzw += buf2_f4.Load(idx2, status);
  r.x += buf2_d1.Load(idx2, status);
  r.xy += buf2_d2.Load(idx2, status);
  r.xyz += buf2_d3.Load(idx2, status);
  r.xyzw += buf2_d4.Load(idx2, status);

  buf2[0].f4 = r;
  return r;
}