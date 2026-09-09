// RUN: %dxc -T ps_6_0 -E main %s -spirv | FileCheck %s

struct PSInput {
  int idx: INPUT0;
  uint uidx: INPUT1;
  nointerpolation      float4 fp_h: FPH;
};

float4 main(PSInput input) : SV_Target {
// CHECK:  [[idx:%[0-9]+]] = OpLoad %int %in_var_INPUT0
// CHECK: [[uidx:%[0-9]+]] = OpLoad %uint %in_var_INPUT1
// CHECK:   [[bc:%[0-9]+]] = OpBitcast %uint [[idx]]
// CHECK:   [[ac:%[0-9]+]] = OpAccessChain %_ptr_Input_v4float %in_var_FPH [[bc]]
// CHECK:    [[a:%[0-9]+]] = OpLoad %v4float [[ac]]
// CHECK:   [[ac:%[0-9]+]] = OpAccessChain %_ptr_Input_v4float %in_var_FPH %21
// CHECK:    [[b:%[0-9]+]] = OpLoad %v4float %25
// CHECK:  [[add:%[0-9]+]] = OpFAdd %v4float [[a]] [[b]]
// CHECK:                    OpStore %out_var_SV_Target [[add]]
  float4 a = GetAttributeAtVertex(input.fp_h, input.idx);
  float4 b = GetAttributeAtVertex(input.fp_h, input.uidx);
  return a+b;
}