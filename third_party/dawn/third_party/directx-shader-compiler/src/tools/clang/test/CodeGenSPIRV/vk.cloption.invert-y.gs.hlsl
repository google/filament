// RUN: %dxc -T gs_6_0 -E main -fvk-invert-y -fcgl  %s -spirv | FileCheck %s

// GS per-vertex input
struct GsVIn {
    float4 pos : SV_Position;
};

// GS per-vertex output
struct GsVOut {
    float4 pos : SV_Position;
};

[maxvertexcount(2)]
void main(in    line GsVIn              inData[2],
          inout      LineStream<GsVOut> outData) {

    GsVOut vertex;
    vertex = (GsVOut)0;
// CHECK:      [[vert:%[0-9]+]] = OpLoad %GsVOut %vertex
// CHECK-NEXT:  [[val:%[0-9]+]] = OpCompositeExtract %v4float [[vert]] 0
// CHECK-NEXT: [[oldY:%[0-9]+]] = OpCompositeExtract %float [[val]] 1
// CHECK-NEXT: [[newY:%[0-9]+]] = OpFNegate %float [[oldY]]
// CHECK-NEXT:  [[pos:%[0-9]+]] = OpCompositeInsert %v4float [[newY]] [[val]] 1
// CHECK-NEXT:                 OpStore %gl_Position_0 [[pos]]
    outData.Append(vertex);

    outData.RestartStrip();
}

