// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpEntryPoint Geometry %main "main"
// CHECK-SAME: %gl_InvocationID

// CHECK:      OpDecorate %gl_InvocationID BuiltIn InvocationId

// CHECK:      %gl_InvocationID = OpVariable %_ptr_Input_uint Input

// GS per-vertex input
struct GsVIn {
    float4 foo : FOO;
};

// GS per-vertex output
struct GsVOut {
    float4 bar : BAR;
};

[maxvertexcount(2)]
void main(in    line GsVIn              inData[2],
                     uint               id         : SV_GSInstanceID,
          inout      LineStream<GsVOut> outData) {

    GsVOut vertex;
    vertex = (GsVOut)0;
    outData.Append(vertex);

    outData.RestartStrip();
}
