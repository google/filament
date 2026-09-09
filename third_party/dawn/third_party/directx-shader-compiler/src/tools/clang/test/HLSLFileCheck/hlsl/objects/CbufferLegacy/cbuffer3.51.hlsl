// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
struct Foo
{
  float4 g1[16];
};

struct Bar
{
  uint3 idx[16];
};

ConstantBuffer<Foo> buf1[32] : register(b77, space3);
ConstantBuffer<Bar> buf2[64] : register(b17);

float4 main(int3 a : A) : SV_TARGET
{
  return buf1[ buf2[a.x].idx[a.y].z ].g1[a.z + 12].wyyy;
}
