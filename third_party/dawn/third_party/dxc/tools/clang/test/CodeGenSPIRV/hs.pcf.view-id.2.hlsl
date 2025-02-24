// RUN: %dxc -T hs_6_1 -E main -fcgl  %s -spirv | FileCheck %s

// Test: Both entry point and PCF Takes ViewID

// CHECK:      OpCapability MultiView
// CHECK:      OpExtension "SPV_KHR_multiview"

// CHECK:      OpEntryPoint TessellationControl
// CHECK-SAME: [[viewindex:%[0-9]+]]

// CHECK:      OpDecorate [[viewindex]] BuiltIn ViewIndex

// CHECK:      [[pcfType:%[0-9]+]] = OpTypeFunction %HsPcfOut %_ptr_Function_uint
// CHECK:         [[viewindex]] = OpVariable %_ptr_Input_uint Input

#define NumOutPoints 2

struct HsCpIn {
    int foo : FOO;
};

struct HsCpOut {
    int bar : BAR;
};

struct HsPcfOut {
  float tessOuter[4] : SV_TessFactor;
  float tessInner[2] : SV_InsideTessFactor;
};

// Patch Constant Function
HsPcfOut pcf(uint viewid : SV_ViewID) {
  HsPcfOut output;
  output = (HsPcfOut)0;
  return output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(NumOutPoints)]
[patchconstantfunc("pcf")]
HsCpOut main(InputPatch<HsCpIn, NumOutPoints> patch,
             uint id : SV_OutputControlPointID) {
    HsCpOut output;
    output = (HsCpOut)0;
    return output;
// CHECK:             %main = OpFunction %void None {{%[0-9]+}}
// CHECK: %param_var_viewid = OpVariable %_ptr_Function_uint Function

// CHECK:      [[val:%[0-9]+]] = OpLoad %uint [[viewindex]]
// CHECK:                     OpStore %param_var_viewid [[val]]
// CHECK:          {{%[0-9]+}} = OpFunctionCall %HsPcfOut %pcf %param_var_viewid

// CHECK:              %pcf = OpFunction %HsPcfOut None [[pcfType]]
// CHECK:           %viewid = OpFunctionParameter %_ptr_Function_uint
}
