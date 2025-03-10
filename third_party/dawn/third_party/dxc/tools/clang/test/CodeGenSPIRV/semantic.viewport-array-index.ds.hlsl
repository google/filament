// RUN: %dxc -T ds_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// HS PCF output
struct HsPcfOut {
  float  outTessFactor[4]   : SV_TessFactor;
  float  inTessFactor[2]    : SV_InsideTessFactor;
  uint   index              : SV_ViewportArrayIndex;
};

// Per-vertex input structs
struct DsCpIn {
  uint   index              : SV_ViewportArrayIndex;
};

// Per-vertex output structs
struct DsCpOut {
  uint   index              : SV_ViewportArrayIndex;
};

// CHECK:      OpEntryPoint TessellationEvaluation %main "main"
// CHECK-SAME: %in_var_SV_ViewportArrayIndex
// CHECK-SAME: %in_var_SV_ViewportArrayIndex_0
// CHECK-SAME: %gl_ViewportIndex

// CHECK:      OpDecorate %gl_ViewportIndex BuiltIn ViewportIndex
// CHECK:      OpDecorate %in_var_SV_ViewportArrayIndex Location 0
// CHECK:      OpDecorate %in_var_SV_ViewportArrayIndex_0 Location 1

// CHECK:      %in_var_SV_ViewportArrayIndex = OpVariable %_ptr_Input__arr_uint_uint_3 Input
// CHECK:      %in_var_SV_ViewportArrayIndex_0 = OpVariable %_ptr_Input_uint Input
// CHECK:      %gl_ViewportIndex = OpVariable %_ptr_Output_uint Output

[domain("quad")]
DsCpOut main(OutputPatch<DsCpIn, 3> patch, HsPcfOut pcfData) {
  DsCpOut dsOut;
  dsOut = (DsCpOut)0;
  return dsOut;
}
