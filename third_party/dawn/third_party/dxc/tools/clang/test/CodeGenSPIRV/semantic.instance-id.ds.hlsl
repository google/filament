// RUN: %dxc -T ds_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// HS PCF output
struct HsPcfOut {
  float  outTessFactor[4]   : SV_TessFactor;
  float  inTessFactor[2]    : SV_InsideTessFactor;
};

// Per-vertex input structs
struct DsCpIn {
    int id : SV_InstanceID;
};

// Per-vertex output structs
struct DsCpOut {
    int id : SV_InstanceID;
};

// CHECK:      OpEntryPoint TessellationEvaluation %main "main"
// CHECK-SAME: %in_var_SV_InstanceID
// CHECK-SAME: %out_var_SV_InstanceID


// CHECK:      OpDecorate %in_var_SV_InstanceID Location 0
// CHECK:      OpDecorate %out_var_SV_InstanceID Location 0

// CHECK:      %in_var_SV_InstanceID = OpVariable %_ptr_Input__arr_int_uint_3 Input
// CHECK:      %out_var_SV_InstanceID = OpVariable %_ptr_Output_int Output

[domain("quad")]
DsCpOut main(OutputPatch<DsCpIn, 3> patch, HsPcfOut pcfData) {
  DsCpOut dsOut;
  dsOut = (DsCpOut)0;
  return dsOut;
}
