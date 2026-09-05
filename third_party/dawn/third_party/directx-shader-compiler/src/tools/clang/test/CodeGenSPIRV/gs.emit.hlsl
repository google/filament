// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct GsInnerOut {
    float2 bar  : BAR;
};

struct GsPerVertexOut {
    float4 pos  : SV_Position;
    float3 foo  : FOO;
    GsInnerOut s;
};

// CHECK: [[null:%[0-9]+]] = OpConstantNull %GsPerVertexOut

[maxvertexcount(2)]
void main(in    line float2 foo[2] : FOO,
          in    line float4 pos[2] : SV_Position,
          inout      LineStream<GsPerVertexOut> outData)
{
// CHECK:            %src_main = OpFunction %void None
// CHECK:            %bb_entry = OpLabel

// CHECK-NEXT:         %vertex = OpVariable %_ptr_Function_GsPerVertexOut Function
    GsPerVertexOut vertex;
// CHECK-NEXT:                   OpStore %vertex [[null]]
    vertex = (GsPerVertexOut)0;

// Write back to stage output variables
// CHECK-NEXT: [[vertex:%[0-9]+]] = OpLoad %GsPerVertexOut %vertex
// CHECK-NEXT:    [[pos:%[0-9]+]] = OpCompositeExtract %v4float [[vertex]] 0
// CHECK-NEXT:                   OpStore %gl_Position_0 [[pos]]
// CHECK-NEXT:    [[foo:%[0-9]+]] = OpCompositeExtract %v3float [[vertex]] 1
// CHECK-NEXT:                   OpStore %out_var_FOO [[foo]]
// CHECK-NEXT:      [[s:%[0-9]+]] = OpCompositeExtract %GsInnerOut [[vertex]] 2
// CHECK-NEXT:    [[bar:%[0-9]+]] = OpCompositeExtract %v2float [[s]] 0
// CHECK-NEXT:                   OpStore %out_var_BAR [[bar]]
// CHECK-NEXT:                   OpEmitVertex

    outData.Append(vertex);

// Write back to stage output variables
// CHECK-NEXT: [[vertex_0:%[0-9]+]] = OpLoad %GsPerVertexOut %vertex
// CHECK-NEXT:    [[pos_0:%[0-9]+]] = OpCompositeExtract %v4float [[vertex_0]] 0
// CHECK-NEXT:                   OpStore %gl_Position_0 [[pos_0]]
// CHECK-NEXT:    [[foo_0:%[0-9]+]] = OpCompositeExtract %v3float [[vertex_0]] 1
// CHECK-NEXT:                   OpStore %out_var_FOO [[foo_0]]
// CHECK-NEXT:      [[s_0:%[0-9]+]] = OpCompositeExtract %GsInnerOut [[vertex_0]] 2
// CHECK-NEXT:    [[bar_0:%[0-9]+]] = OpCompositeExtract %v2float [[s_0]] 0
// CHECK-NEXT:                   OpStore %out_var_BAR [[bar_0]]
// CHECK-NEXT:                   OpEmitVertex
    outData.Append(vertex);

// CHECK-NEXT:                   OpEndPrimitive
    outData.RestartStrip();

// CHECK-NEXT:                   OpReturn
}
