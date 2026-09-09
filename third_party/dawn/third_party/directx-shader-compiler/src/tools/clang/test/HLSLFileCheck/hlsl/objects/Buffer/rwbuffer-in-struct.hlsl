// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: ; Note: shader requires additional functionality:
// CHECK: ;       UAVs at every shader stage
// CHECK: ;       Typed UAV Load Additional Formats

// CHECK: call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32

struct S {
  RWBuffer<float4> buf;
} s;

float4 main() : OUT {
  return s.buf[0];
}
