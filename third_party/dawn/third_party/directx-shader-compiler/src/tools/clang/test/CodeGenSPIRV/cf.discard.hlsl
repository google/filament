// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Note: The proper way to translate HLSL 'discard' to SPIR-V is using the
// 'OpDemoteToHelperInvocationEXT' instruction, which requires the
// SPV_EXT_demote_to_helper_invocation extension.
//
// If you want that behavior, please use the following command line option:
// '-fspv-extension=SPV_EXT_demote_to_helper_invocation'
//
// In the absence of this option DXC will generate 'OpKill' for 'discard'.

void main() {
  int a, b;
  bool cond = true;

  while(cond) {
// CHECK: %while_body = OpLabel
    if(a==b) {
// CHECK: %if_true = OpLabel
// CHECK-NEXT: OpKill
      {{discard;}}
      discard;  // No SPIR-V should be emitted for this statement.
      break;    // No SPIR-V should be emitted for this statement.
    } else {
// CHECK-NEXT: %if_false = OpLabel
      ++a;
// CHECK: OpKill
      discard;
      continue; // No SPIR-V should be emitted for this statement.
      --b;      // No SPIR-V should be emitted for this statement.
    }
// CHECK-NEXT: %if_merge = OpLabel

  }
// CHECK: %while_merge = OpLabel

}
