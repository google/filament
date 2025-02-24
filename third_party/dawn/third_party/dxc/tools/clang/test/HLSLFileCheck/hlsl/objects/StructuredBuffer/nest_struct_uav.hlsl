// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure nest struct type works for UAV.
// CHECK: @dx.op.bufferLoad.i32
// CHECK: dx.op.bufferStore.i32

struct Foo
{
  uint a;
};

struct Bar {
  Foo f;
  float b;
};

StructuredBuffer<Bar> buf1;
RWStructuredBuffer<Bar> buf2;


float4 main(float idx1 : Idx1, float idx2 : Idx2, int2 c : C) : SV_Target
{
  Foo f = buf1.Load(idx1).f;
  buf2[idx1*3].f = f;

  return 1;
}