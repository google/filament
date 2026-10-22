// RUN: %dxc -T ms_6_5 -E main -fspv-target-env=vulkan1.1spirv1.4 -fspv-extension=SPV_NV_mesh_shader %s -spirv | FileCheck %s
// CHECK:  OpCapability MeshShadingNV
// CHECK:  OpExtension "SPV_NV_mesh_shader"
// CHECK:  OpEntryPoint MeshNV %main "main" %gl_Position [[primind:%[0-9]+]] [[primcount:%[0-9]+]]
// CHECK:  OpExecutionMode %main LocalSize 32 1 1
// CHECK:  OpExecutionMode %main OutputTrianglesEXT
// CHECK:  OpExecutionMode %main OutputVertices 81
// CHECK:  OpExecutionMode %main OutputPrimitivesEXT 128

// CHECK:  OpDecorate %gl_Position BuiltIn Position
// CHECK:  OpDecorate [[primind]] BuiltIn PrimitiveIndicesNV
// CHECK:  OpDecorate [[primcount]] BuiltIn PrimitiveCountNV

// Make sure that when spirv 1.4 or above is supported, but we explicitly request SPV_NV_mesh_shader through -fpv-extension,
// that we output SPIR-V using SPV_NV_mesh_shader and don't override using the EXT extension

struct MeshPerVertex {
    float4 position : SV_Position;
};

#define NUM_THREADS 32
#define MAX_VERT 81
#define MAX_PRIM 128

[outputtopology("triangle")]
[numthreads(NUM_THREADS, 1, 1)]
void main(out vertices MeshPerVertex verts[MAX_VERT],
          out indices uint3 primitiveInd[MAX_PRIM],
          in uint tid : SV_DispatchThreadID)
{
    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);
}
