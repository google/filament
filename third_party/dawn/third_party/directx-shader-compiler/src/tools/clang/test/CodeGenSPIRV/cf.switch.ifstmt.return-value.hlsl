// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

cbuffer cb {
  uint   index;
  float4 f0;
  float4 f1;
};


float4 fallthrough_switch(uint id) {
// CHECK: %fallthrough_switch = OpFunction

  switch (id) {
// CHECK:    [[id:%[0-9]+]] = OpLoad %uint %id
// CHECK:                     OpSwitch [[id]] %switch_default 0 %switch_0 1 %switch_1

    case 0:
      return f0;
// CHECK:          %switch_0 = OpLabel
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %cb %int_1
// CHECK:    [[val:%[0-9]+]] = OpLoad %v4float [[ptr]]
// CHECK:                      OpReturnValue [[val]]

    case 1:
    default:
      return f1;
// CHECK:          %switch_1 = OpLabel
// CHECK:                      OpBranch %switch_default
// CHECK:    %switch_default = OpLabel
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %cb %int_2
// CHECK:    [[val:%[0-9]+]] = OpLoad %v4float [[ptr]]
// CHECK:                      OpReturnValue [[val]]
  }
}

float4 fallthrough_branch(uint id) {
// CHECK: %fallthrough_branch = OpFunction

  [branch]
  switch (id) {
// CHECK:     [[id:%[0-9]+]] = OpLoad %uint %id_0
// CHECK:   [[cond:%[0-9]+]] = OpIEqual %bool [[id]] %uint_0
// CHECK:                      OpBranchConditional [[cond]] %if_true %if_false

    case 0:
      return f0;
// CHECK:           %if_true = OpLabel
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %cb %int_1
// CHECK:    [[val:%[0-9]+]] = OpLoad %v4float [[ptr]]
// CHECK:                      OpReturnValue [[val]]

    case 1:
    default:
      return f1;
// CHECK:          %if_false = OpLabel
// CHECK:     [[id:%[0-9]+]] = OpLoad %uint %id_0
// CHECK:   [[cond:%[0-9]+]] = OpIEqual %bool [[id]] %uint_1
// CHECK:                      OpBranchConditional [[cond]] %if_true_0 %if_false_0

// CHECK:         %if_true_0 = OpLabel
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %cb %int_2
// CHECK:    [[val:%[0-9]+]] = OpLoad %v4float [[ptr]]
// CHECK:                      OpReturnValue [[val]]

// CHECK:        %if_false_0 = OpLabel
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %cb %int_2
// CHECK:    [[val:%[0-9]+]] = OpLoad %v4float [[ptr]]
// CHECK:                      OpReturnValue [[val]]
  }
}

float4 main() : SV_Target
{
  float4 a = fallthrough_switch(index);
  float4 b = fallthrough_branch(index);
  return a + b;
}
