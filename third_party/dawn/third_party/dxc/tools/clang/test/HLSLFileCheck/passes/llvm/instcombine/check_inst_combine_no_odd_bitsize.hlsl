// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// Make sure instcombine doesn't introduce odd int bitsize
// from shift+cmp -> trunc+cmp optimiation.

// CHECK: define void @main()
// CHECK-NOT: i29

uint main(uint i : A) : SV_Target
{
  if ((i * 8) < 16)
    return 1;
  return i;
}
