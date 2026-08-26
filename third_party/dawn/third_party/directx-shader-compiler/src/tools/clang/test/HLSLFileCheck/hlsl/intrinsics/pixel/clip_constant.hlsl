// RUN: %dxc -E main_discard -T ps_6_0 %s | FileCheck -check-prefix=DSCRD %s
// RUN: %dxc -E main_no_discard -T ps_6_0 %s | FileCheck -check-prefix=NODSCRD %s

// Test that when input to clip() is a compile-time constant,
// a discard instruction is not emitted when its input is non-negative like FXC.

// Github issue# 2515

float4 main_discard() : SV_Target
{
  // DSCRD: dx.op.discard
  clip(-1);
  
  // DSCRD: dx.op.discard
  clip(float4(-0.001, 10, 1, 0));
  
  return 0;
}

float4 main_no_discard() : SV_Target
{
  // NODSCRD-NOT: dx.op.discard
  clip(float4(1, 0, 0, 1));
  
  // NODSCRD-NOT: dx.op.discard
  clip(float3(0, 0, 0));
  
  // NODSCRD-NOT: dx.op.discard
  clip(float2(0.001, 0.00001));
  
  // NODSCRD-NOT: dx.op.discard
  clip(0);
  
  // NODSCRD-NOT: dx.op.discard
  clip(1);
  
  return 0;
}


