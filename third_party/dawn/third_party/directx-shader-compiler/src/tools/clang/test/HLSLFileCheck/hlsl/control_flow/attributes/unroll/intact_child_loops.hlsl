// RUN: %dxc -Od -E main -T ps_6_0 %s | FileCheck %s
// CHECK: @main

// Test case for when an unrolled loop has a child loop that's not unrolled.
// regression test for GitHub #2006

void main()
{
  for (int i = 0; i < 1; i++)
    [unroll]
    for (int j = 0; j < 1; j++)
      for (int k = 0; k < 1; k++)
        ;
}
