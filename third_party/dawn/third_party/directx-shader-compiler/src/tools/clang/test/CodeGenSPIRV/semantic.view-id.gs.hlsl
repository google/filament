// RUN: %dxc -T gs_6_1 -E main -fcgl  %s -spirv | FileCheck %s


// GS per-vertex input
struct GsVIn {
    int foo : FOO;
};

// GS per-vertex output
struct GsVOut {
    int foo : FOO;
};

// CHECK:      OpCapability MultiView
// CHECK:      OpExtension "SPV_KHR_multiview"

// CHECK:      OpEntryPoint Geometry
// CHECK-SAME: [[viewindex:%[0-9]+]]

// CHECK:      OpDecorate [[viewindex]] BuiltIn ViewIndex

// CHECK:      [[viewindex]] = OpVariable %_ptr_Input_uint Input


[maxvertexcount(2)]
void main(in    line GsVIn              inData[2],
          inout      LineStream<GsVOut> outData,
                     uint               viewid  : SV_ViewID) {

    GsVOut vertex;
    vertex = (GsVOut)0;
    outData.Append(vertex);

    outData.RestartStrip();
}
