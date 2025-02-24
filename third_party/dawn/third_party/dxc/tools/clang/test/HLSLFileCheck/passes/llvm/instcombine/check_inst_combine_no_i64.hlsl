// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// Make sure instcombine doesn't introduce i64 from FoldCmpLoadFromIndexedGlobal.

// CHECK: define void @main()
// CHECK-NOT: i64

static const float A[33] = {
  -1.0,
  -1.0,
   1.0,
   1.0,
   1.0,
   1.0,
  -1.0,
  -1.0,
  -1.0,
  -1.0,
  -1.0,
  -1.0,
  -1.0,
  -1.0,
  -1.0,
   1.0,
   1.0,
   1.0,
   1.0,
   1.0,
   1.0,
  -1.0,
  -1.0,
  -1.0,
   1.0,
   1.0,
   1.0,
   1.0,
   1.0,
   1.0,
  -1.0,
  -1.0,
  -1.0,
};


[RootSignature("DescriptorTable(SRV(t0),SRV(t1, numDescriptors=9))")]
uint main(uint i : A) : SV_Target
{
  float f = A[i];
  if (f < 0.0)
  {
    return 17;
  }
  else
  {
    return 23;
  }
}
