// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: RWStructuredBuffers may increment or decrement their counters, but not both.

struct Foo
{
  float2 a;
  float3 b;
  int2 c[4];
};

Buffer<float4> buf1;
RWStructuredBuffer<Foo> buf2;

float4 main(float idx1 : Idx1, float idx2 : Idx2) : SV_Target
{
  uint status;
  float4 r = 0;
  int id = buf2.IncrementCounter();
  buf2[id].a = float2(idx1, idx2);

  id = buf2.IncrementCounter();
  r.xy += buf1[id].a;  
  
  return r;
}
