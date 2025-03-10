// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: r/w+cnt
// CHECK: r/w+cnt
// CHECK: r/w+cnt
// CHECK: r/w+cnt


struct Foo
{
  float2 a;
  float3 b;
  int2 c[4];
};

AppendStructuredBuffer<Foo> buf1;
RWStructuredBuffer<Foo> buf2;
ConsumeStructuredBuffer<Foo> buf3;
RWStructuredBuffer<Foo> buf4;

float4 main(float idx1 : Idx1, float idx2 : Idx2) : SV_Target
{
  float4 r = 0;
  uint counter = buf2.IncrementCounter();
  counter = buf4.DecrementCounter();
  Foo f = buf3.Consume();
  buf1.Append(f);
  buf2[counter] = buf4[counter];
  return r;
}
