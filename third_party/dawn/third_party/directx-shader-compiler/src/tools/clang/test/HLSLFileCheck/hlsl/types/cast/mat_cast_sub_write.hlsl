// RUN: %dxc /T ps_6_0 /E main %s | FileCheck %s

// Make sure cast then subscript works.

float4x4 a;

struct B3 {
  uint3 ui;
};

struct B {
   uint4  ui;
};

struct M {
  struct B base;
  float4x3 a;
  float4 b;
  float4 c;
  float  d[6];
};

struct B3 b;

RWStructuredBuffer<M> buf;

float3 main(uint i:I, float3 x:X) :SV_Target {
  // Make sure match no cast version.

  // CHECK:call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %buf_UAV_structbuf, i32 %{{[0-9]+}}, i32 64, float 3.000000e+00, float undef, float undef, float undef, i8 1)

  ((float)buf[i].b) = 3;
  // CHECK:call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %buf_UAV_structbuf, i32 %{{[0-9]+}}, i32 80, float 1.000000e+01, float 9.000000e+00, float 8.000000e+00, float undef, i8 7)
  ((float3)buf[i].c) = float3(10,9,8);

  // CHECK:call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %buf_UAV_structbuf, i32 %{{[0-9]+}}, i32 96, float 5.000000e+00, float undef, float undef, float undef, i8 1)
  // CHECK:call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %buf_UAV_structbuf, i32 %{{[0-9]+}}, i32 100, float 6.000000e+00, float undef, float undef, float undef, i8 1)
  // CHECK:call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %buf_UAV_structbuf, i32 %{{[0-9]+}}, i32 104, float 7.000000e+00, float undef, float undef, float undef, i8 1)
  float td[3] = {5,6,7};
  ((float[3])buf[i].d) = td;

  //CHECK:call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %buf_UAV_structbuf, i32 %{{[0-9]+}}, i32 0, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 undef, i8 7)
  ((B3)buf[i].base) = b;

  // buf[i].a[1].xyz = x;
  // CHECK:call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %buf_UAV_structbuf, i32 %{{[0-9]+}}, i32 20
  // CHECK:call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %buf_UAV_structbuf, i32 %{{[0-9]+}}, i32 36
  // CHECK:call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %buf_UAV_structbuf, i32 %{{[0-9]+}}, i32 52

  ((float3x3)buf[i].a)[1] = x;
  // a[1].xyz + a._m21_m20_m02;
  // CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK:extractvalue %dx.types.CBufRet.f32 %{{[0-9]+}}, 1
  // CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK:extractvalue %dx.types.CBufRet.f32 %{{[0-9]+}}, 1
  // CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK:extractvalue %dx.types.CBufRet.f32 %{{[0-9]+}}, 1
  // CHECK:extractvalue %dx.types.CBufRet.f32 %{{[0-9]+}}, 2
  // CHECK:extractvalue %dx.types.CBufRet.f32 %{{[0-9]+}}, 2
  // CHECK:extractvalue %dx.types.CBufRet.f32 %{{[0-9]+}}, 0
  return ((float3x3)a)[1] + ((float3x3)a)._m21_m20_m02;
}