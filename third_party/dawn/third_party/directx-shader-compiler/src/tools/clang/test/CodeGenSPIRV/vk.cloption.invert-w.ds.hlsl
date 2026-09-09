// RUN: %dxc -T ds_6_0 -E main -fvk-use-dx-position-w -fcgl  %s -spirv | FileCheck %s

// HS PCF output
struct HsPcfOut {
  float  outTessFactor[4]   : SV_TessFactor;
  float  inTessFactor[2]    : SV_InsideTessFactor;
};

// Per-vertex input structs
struct DsCpIn {
    float4 pos : SV_Position;
};

// Per-vertex output structs
struct DsCpOut {
    float4 pos : SV_Position;
};

[domain("quad")]
DsCpOut main(OutputPatch<DsCpIn, 3> patch,
             HsPcfOut pcfData) {
  DsCpOut dsOut;
  dsOut = (DsCpOut)0;
  return dsOut;
}

// Make sure -fvk-use-dx-position-w is ignored for non-PS stages
// CHECK:     OpLoad %_arr_v4float_uint_3 %gl_Position
// CHECK-NOT: OpCompositeInsert %v4float {{%[0-9]+}} {{%[0-9]+}} 3
