// RUN: %dxc -T hs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

#define NumOutPoints 2

// CHECK:      OpEntryPoint TessellationControl %main "main"
// CHECK-SAME: %in_var_SV_InstanceID
// CHECK-SAME: %out_var_SV_InstanceID

// CHECK:      OpDecorate %in_var_SV_InstanceID Location 0
// CHECK:      OpDecorate %out_var_SV_InstanceID Location 0

// CHECK:      %in_var_SV_InstanceID = OpVariable %_ptr_Input__arr_int_uint_2 Input
// CHECK:      %out_var_SV_InstanceID = OpVariable %_ptr_Output__arr_int_uint_2 Output

struct HsCpIn {
    int id : SV_InstanceID;
};

struct HsCpOut {
    int id : SV_InstanceID;
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
             uint id : SV_OutputControlPointID) {
    HsCpOut output;
    output = (HsCpOut)0;
    return output;
}
