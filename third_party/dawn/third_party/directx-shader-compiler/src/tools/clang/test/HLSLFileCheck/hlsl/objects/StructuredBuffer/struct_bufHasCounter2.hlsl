// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: r/w+cnt
// CHECK: r/w+cnt
// CHECK: r/w+cnt
// CHECK: r/w+cnt


AppendStructuredBuffer<uint> buf1;
RWStructuredBuffer<uint> buf2;
ConsumeStructuredBuffer<uint> buf3;
RWStructuredBuffer<uint> buf4;

float4 main(float idx1 : Idx1, float idx2 : Idx2) : SV_Target
{
  float4 r = 0;
  uint counter = buf2.IncrementCounter();
  counter = buf4.DecrementCounter();
  uint f = buf3.Consume();
  buf1.Append(f);
  buf2[counter] = buf4[counter];
  return r;
}
