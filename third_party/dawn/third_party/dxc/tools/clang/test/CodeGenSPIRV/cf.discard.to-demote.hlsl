// RUN: %dxc -T ps_6_0 -E main -fspv-extension=SPV_EXT_demote_to_helper_invocation -fcgl  %s -spirv | FileCheck %s

// According to the HLS spec, discard can only be called from a pixel shader.

// CHECK: OpCapability DemoteToHelperInvocation
// CHECK: OpExtension "SPV_EXT_demote_to_helper_invocation"

void main() {
  int a, b;
  bool cond = true;

  while(cond) {
// CHECK: %while_body = OpLabel
    if(a==b) {
// CHECK: %if_true = OpLabel
// CHECK: OpDemoteToHelperInvocation
      discard;
      break;
    } else {
// CHECK: %if_false = OpLabel
      ++a;
// CHECK: OpDemoteToHelperInvocation
      discard;
      continue;
      --b;
    }
// CHECK: %if_merge = OpLabel
  }
// CHECK: %while_merge = OpLabel

}
