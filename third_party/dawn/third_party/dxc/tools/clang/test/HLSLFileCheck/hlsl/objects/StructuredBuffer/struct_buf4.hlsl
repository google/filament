// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -enable-16bit-types %s | FileCheck %s
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 4, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 12, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 24, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 40, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 44, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 52, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 64, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 80, i8 1, i32 2)
// CHECK: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 82, i8 3, i32 2)
// CHECK: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 86, i8 7, i32 2)
// CHECK: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 92, i8 15, i32 2)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 100, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 104, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 112, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 124, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 144, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 152, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 168, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 184, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 192, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 208, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 224, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 236, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 248, i8 1, i32 4)

// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 0, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 4, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 12, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 24, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 40, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 44, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 52, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 64, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 80, i8 1, i32 2)
// CHECK: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 82, i8 3, i32 2)
// CHECK: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 86, i8 7, i32 2)
// CHECK: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 92, i8 15, i32 2)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 100, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 104, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 112, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 124, i8 15, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 144, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 152, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 168, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 184, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 192, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 208, i8 15, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 224, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 236, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_structbuf, i32 %{{[a-zA-Z0-9]+}}, i32 248, i8 1, i32 4)

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
  int3x3 i3x3;
};
StructuredBuffer<MyStruct> buf1;
RWStructuredBuffer<MyStruct> buf2;
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
  r.xyz += buf1.Load(idx1, status).i3x3[0];

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
  r.xyz += buf2.Load(idx2, status).i3x3[0];

  return r;
}