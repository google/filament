// RUN: %dxc %s -T ps_6_0 -Od | FileCheck %s

// This test verifies the fix for a deficiency in RemoveDeadBlocks where:
//
// - Value 'ret' that can be reduced to constant by DxilValueCache is removed
// - It held on uses for a PHI 'val', but 'val' was not removed
// - 'val' is not used, but also not DCE'ed until after DeleteDeadRegion is run
// - DeleteDeadRegion cannot delete 'if (foo)' because 'val' still exists.

// CHECK: @main
// CHECK-NOT: phi

cbuffer cb : register(b0) {
  float foo;
}

[RootSignature("")]
float main() : SV_Target {
  float val = 0;
  if (foo)
    val = 1;

  float zero = 0;
  float ret = val * zero;

  return ret;
}
