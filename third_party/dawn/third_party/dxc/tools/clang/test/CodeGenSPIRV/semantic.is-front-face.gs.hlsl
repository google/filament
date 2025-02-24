// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpEntryPoint Geometry %main "main"
// CHECK-SAME: %out_var_SV_IsFrontFace

// CHECK:      OpDecorate %out_var_SV_IsFrontFace Location 0

// CHECK:      %out_var_SV_IsFrontFace = OpVariable %_ptr_Output_uint Output

// GS per-vertex input
struct GsVIn {
    float4 foo : FOO;
};

// GS per-vertex output
struct GsVOut {
    bool frontFace : SV_IsFrontFace;
};

[maxvertexcount(2)]
void main(in    line GsVIn              inData[2],
          inout      LineStream<GsVOut> outData) {

    GsVOut vertex;
    vertex = (GsVOut)0;
    outData.Append(vertex);

    outData.RestartStrip();
}
