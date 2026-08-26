// RUN: %dxc -T ds_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// HS PCF output
struct HsPcfOut {
  float  outTessFactor[4]   : SV_TessFactor;
  float  inTessFactor[2]    : SV_InsideTessFactor;
};

// Per-vertex input structs
struct DsCpIn {
    float4 foo : FOO;
};

// Per-vertex output structs
struct DsCpOut {
    float4 bar : BAR;
};

// CHECK:      OpEntryPoint TessellationEvaluation %main "main"
// CHECK-SAME: %gl_PrimitiveID

// CHECK:      OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId

// CHECK:      %gl_PrimitiveID = OpVariable %_ptr_Input_uint Input

[domain("quad")]
DsCpOut main(OutputPatch<DsCpIn, 3> patch, HsPcfOut pcfData, uint id : SV_PrimitiveID) {
  DsCpOut dsOut;
  dsOut = (DsCpOut)0;
  return dsOut;
}
