// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s


// Tests vector index on buffer load.
// CHECK:alloca [4 x float]

// Tests NonUniformResourceIndex.
// CHECK:dx.op.createHandle(i32 57, i8 0, i32 0, i32 {{.*}}, i1 true

// CHECK:dx.op.bufferLoad.f32
// CHECK:store
// CHECK:store
// CHECK:store
// CHECK:store
// CHECK:load

Buffer<float4> buf[10];

float4 main(float b : B, uint i:I) : SV_Target {
  uint idx = NonUniformResourceIndex(i);
  return buf[idx][idx][idx];
}