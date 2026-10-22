// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Note that 'Raw and Structured buffers' is derived from feature part
// in container, which is only present for CS 4.x

// CHECK: 64 UAV slots
// CHECK: Typed UAV Load Additional Formats
// CHECK: Raster Ordered UAVs

struct AStruct {
  float4 f4;
  uint4 u4;
};

RasterizerOrderedBuffer<float4>            rob  : register(u0);
RasterizerOrderedByteAddressBuffer         rba  : register(u1);
RasterizerOrderedStructuredBuffer<AStruct> rsb  : register(u2);
RasterizerOrderedTexture1D<float4>         rt1  : register(u3);
RasterizerOrderedTexture1DArray<float4>    rt1a : register(u4);
RasterizerOrderedTexture2D<float4>         rt2  : register(u5);
RasterizerOrderedTexture2DArray<float4>    rt2a : register(u6);
RasterizerOrderedTexture3D<float4>         rt3  : register(u7);
RasterizerOrderedTexture3D<float4>         rt4  : register(u8);

// CHECK: main
float4 main() : SV_TARGET {
// CHECK: rt3_UAV_3d_ROV
// CHECK: rt2a_UAV_2darray_ROV
// CHECK: rt2_UAV_2d_ROV
// CHECK: rt1a_UAV_1darray_ROV
// CHECK: rt1_UAV_1d_ROV
// CHECK: rsb_UAV_structbuf_ROV
// CHECK: rba_UAV_rawbuf_ROV
// CHECK: rob_UAV_buf_ROV

  float4 result = 0;
// CHECK: dx.op.bufferLoad.f32(i32 68,
  result += rob[0];
// CHECK: dx.op.bufferLoad.i32(i32 68
  result += rba.Load(0);
// CHECK: dx.op.bufferLoad.f32(i32 68,
  result += rsb[0].f4;
// CHECK: dx.op.textureLoad.f32(i32 66,
  result += rt1[0];
// CHECK: dx.op.textureLoad.f32(i32 66,
  result += rt1a[uint2(0, 0)];
// CHECK: dx.op.textureLoad.f32(i32 66,
  result += rt2[uint2(0, 1)];
// CHECK: dx.op.textureLoad.f32(i32 66,
  result += rt2a[uint3(0, 0, 0)];
// CHECK: dx.op.textureLoad.f32(i32 66,
  result += rt3[uint3(1, 2, 3)];

  result += rt4[uint3(1, 2, 3)];
  return result;
}
