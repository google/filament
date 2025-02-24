// RUN: %dxc -T ms_6_5 -E main -fspv-target-env=vulkan1.1spirv1.4 %s -spirv | FileCheck %s
// CHECK:  OpCapability MeshShadingEXT
// CHECK:  OpExtension "SPV_EXT_mesh_shader"
// CHECK:  OpEntryPoint MeshEXT %main "main" %gl_Position [[primind:%[0-9]+]]
// CHECK:  OpExecutionMode %main LocalSize 64 1 1
// CHECK:  OpExecutionMode %main OutputTrianglesEXT
// CHECK:  OpExecutionMode %main OutputVertices 81
// CHECK:  OpExecutionMode %main OutputPrimitivesEXT 128

// CHECK:  OpDecorate %gl_Position BuiltIn Position
// CHECK:  OpDecorate [[primind]] BuiltIn PrimitiveTriangleIndicesEXT

// Make sure that when spirv 1.4 or above is supported, and no extensions are explicitly requested, that
// we output SPIR-V using SPV_EXT_mesh_shader as the default mesh extension

struct MeshPerVertex {
    float4 position : SV_Position;
};

#define NUM_THREADS 64
#define MAX_VERT 81
#define MAX_PRIM 128

[outputtopology("triangle")]
[numthreads(NUM_THREADS, 1, 1)]
void main(out vertices MeshPerVertex verts[MAX_VERT],
          out indices uint3 primitiveInd[MAX_PRIM],
          in uint tid : SV_DispatchThreadID)
{
// CHECK:  OpSetMeshOutputsEXT %uint_81 %uint_128
    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);
}
