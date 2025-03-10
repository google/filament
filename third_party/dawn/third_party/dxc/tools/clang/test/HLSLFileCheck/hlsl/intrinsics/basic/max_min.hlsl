// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: FMax
// CHECK: FMin
// CHECK: IMax
// CHECK: IMin
// CHECK: UMax
// CHECK: UMin


float4 fa;
float4 fb;
int4   ia;
int4   ib;
uint4  ua;
uint4  ub;

float4 main(float4 a : A) : SV_TARGET
{
  return max(fa,fb) + min(fa, fb) + max(ia,ib) + min(ia,ib) + max(ua,ub) + min(ua, ub);
}

