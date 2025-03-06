// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s

// Make sure use imax for iabs.
// CHECK: IMax

int4 main(int4 a : A) : SV_TARGET
{
  return abs(a.yxxx);
}