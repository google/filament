// RUN: %dxc -T ps_6_0 -E main -fspv-target-env=vulkan1.3 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability DemoteToHelperInvocation

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
