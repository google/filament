// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_0 -Od %s | FileCheck %s -check-prefix=CHECKOD

// CHECK: @dx.op.textureLoad.f32(i32 66,
// CHECK: @dx.op.textureLoad.f32(i32 66,
// CHECK-NOT: @dx.op.textureLoad.f32(i32 66,

// CHECKOD: @dx.op.textureLoad.f32(i32 66,
// CHECKOD: @dx.op.textureLoad.f32(i32 66,
// CHECKOD: @dx.op.textureLoad.f32(i32 66,
// CHECKOD: @dx.op.textureLoad.f32(i32 66,
// CHECKOD: @dx.op.textureLoad.f32(i32 66,
// CHECKOD: @dx.op.textureLoad.f32(i32 66,

Texture2D<float4> A, B;

struct ResStruct {
  Texture2D<float4> arr[2];
};

static ResStruct RS = { { A, B } };
static const ResStruct RS_const = { { A, B } };

float4 main() : OUT {
  float4 result = RS.arr[1].Load(uint3(0,0,0))
                + RS_const.arr[1].Load(uint3(0,0,0));
  [unroll]
  for (uint i = 0; i < 2; i++)
    result += RS.arr[i].Load(uint3(0,0,0))
            + RS_const.arr[i].Load(uint3(0,0,0));
  return result;
}
