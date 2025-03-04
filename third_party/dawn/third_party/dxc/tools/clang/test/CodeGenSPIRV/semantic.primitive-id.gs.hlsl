// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpEntryPoint Geometry %main "main"
// CHECK-SAME: %gl_PrimitiveID
// CHECK-SAME: %gl_PrimitiveID_0

// CHECK:      OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
// CHECK:      OpDecorate %gl_PrimitiveID_0 BuiltIn PrimitiveId

// CHECK:      %gl_PrimitiveID = OpVariable %_ptr_Input_uint Input
// CHECK:      %gl_PrimitiveID_0 = OpVariable %_ptr_Output_uint Output

// GS per-vertex input
struct GsVIn {
    float4 foo : FOO;
};

// GS per-vertex output
struct GsVOut {
    uint outId : SV_PrimitiveID;
};

[maxvertexcount(2)]
void main(in    line GsVIn              inData[2],
                     uint               inId     : SV_PrimitiveID,
          inout      LineStream<GsVOut> outData) {

    GsVOut vertex;
    vertex = (GsVOut)0;
    outData.Append(vertex);

    outData.RestartStrip();
}
