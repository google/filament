// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck %s

// CHECK: @main
uint4 a;
uint2 main() : SV_TARGET
{
  if (all(a? 1 : 0))
    return 0;
  else
    return 1;
}
