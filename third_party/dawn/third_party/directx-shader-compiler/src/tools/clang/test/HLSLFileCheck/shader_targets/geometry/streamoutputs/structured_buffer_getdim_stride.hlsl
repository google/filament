// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// CHECK: ret i32 20

struct Foo
{
    float4 a;
    uint b;
};

RWStructuredBuffer<Foo> g_buffer[2] : register(u0);

uint GetStride(int i) {
  RWStructuredBuffer<Foo> buf;
  if (i < 0) {
    buf = g_buffer[0];
  } else {
    buf = g_buffer[1];
  }
  uint a = 0;
  uint elementStride = 0;
  buf.GetDimensions(a, elementStride);
  return elementStride;
}
