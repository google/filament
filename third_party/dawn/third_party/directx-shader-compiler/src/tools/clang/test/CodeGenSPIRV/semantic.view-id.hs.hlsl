// RUN: %dxc -T hs_6_1 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability MultiView
// CHECK:      OpExtension "SPV_KHR_multiview"

// CHECK:      OpEntryPoint TessellationControl
// CHECK-SAME: [[viewindex:%[0-9]+]]

// CHECK:      OpDecorate [[viewindex]] BuiltIn ViewIndex

// CHECK:      [[viewindex]] = OpVariable %_ptr_Input_uint Input

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
HsPcfOut pcf() {
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
             uint id : SV_OutputControlPointID,
             uint viewid : SV_ViewID) {
    HsCpOut output;
    output = (HsCpOut)0;
    return output;
}
