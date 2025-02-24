// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpEntryPoint Geometry %main "main"
// CHECK-SAME: %in_var_SV_InstanceID
// CHECK-SAME: %out_var_SV_InstanceID


// CHECK:      OpDecorate %in_var_SV_InstanceID Location 0
// CHECK:      OpDecorate %out_var_SV_InstanceID Location 0

// CHECK:      %in_var_SV_InstanceID = OpVariable %_ptr_Input__arr_int_uint_2 Input
// CHECK:      %out_var_SV_InstanceID = OpVariable %_ptr_Output_int Output

// GS per-vertex input
struct GsVIn {
    int id : SV_InstanceID;
};

// GS per-vertex output
struct GsVOut {
    int id : SV_InstanceID;
};

[maxvertexcount(2)]
void main(in    line GsVIn              inData[2],
          inout      LineStream<GsVOut> outData) {

    GsVOut vertex;
    vertex = (GsVOut)0;
    outData.Append(vertex);

    outData.RestartStrip();
}
