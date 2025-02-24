// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Minimum-precision data types

// CHECK: add nsw i16
// CHECK: icmp eq i16
// CHECK: add nsw i16
// CHECK: sitofp i16
// CHECK: to float

min16int g1;

float main(min16int a : A) : SV_Target
{
    min16int q = a + 2;
    [branch]
    if (q == g1)
      return q - 3;
    else
      return 1; 
}
