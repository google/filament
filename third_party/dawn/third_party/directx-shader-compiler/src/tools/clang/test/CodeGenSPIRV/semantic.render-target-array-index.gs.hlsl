// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability Geometry

// CHECK:      OpEntryPoint Geometry %main "main"
// CHECK-SAME: %in_var_SV_RenderTargetArrayIndex
// CHECK-SAME: %gl_Layer

// CHECK:      OpDecorate %gl_Layer BuiltIn Layer
// CHECK:      OpDecorate %in_var_SV_RenderTargetArrayIndex Location 0

// CHECK:      %in_var_SV_RenderTargetArrayIndex = OpVariable %_ptr_Input__arr_uint_uint_2 Input
// CHECK:      %gl_Layer = OpVariable %_ptr_Output_uint Output

// GS per-vertex input
struct GsVIn {
  uint index : SV_RenderTargetArrayIndex;
};

// GS per-vertex output
struct GsVOut {
  uint index : SV_RenderTargetArrayIndex;
};

[maxvertexcount(2)]
void main(in    line GsVIn              inData[2],
          inout      LineStream<GsVOut> outData) {

    GsVOut vertex;
    vertex = (GsVOut)0;
    outData.Append(vertex);

    outData.RestartStrip();
}
