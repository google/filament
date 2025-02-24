// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Minimum-precision data types
// CHECK: icmp eq i16

float main(min16int a : A) : SV_Target
{
    min16int q = a + 2;
    [branch]
    if (q == min16int(7))
      return q - 3;
    else
      return 1; 
}
