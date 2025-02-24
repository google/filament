// RUN: %dxc -T ds_6_1 -E main -fcgl  %s -spirv | FileCheck %s

// HS PCF output
struct HsPcfOut {
  float  outTessFactor[4]   : SV_TessFactor;
  float  inTessFactor[2]    : SV_InsideTessFactor;
};

// Per-vertex input structs
struct DsCpIn {
    int foo : FOO;
};

// Per-vertex output structs
struct DsCpOut {
    int foo : FOO;
};

// CHECK:      OpCapability MultiView
// CHECK:      OpExtension "SPV_KHR_multiview"

// CHECK:      OpEntryPoint TessellationEvaluation
// CHECK-SAME: [[viewindex:%[0-9]+]]

// CHECK:      OpDecorate [[viewindex]] BuiltIn ViewIndex

// CHECK:      [[viewindex]] = OpVariable %_ptr_Input_uint Input


[domain("quad")]
DsCpOut main(OutputPatch<DsCpIn, 3> patch,
             HsPcfOut pcfData,
             uint viewid : SV_ViewID) {
  DsCpOut dsOut;
  dsOut = (DsCpOut)0;
  return dsOut;
}
