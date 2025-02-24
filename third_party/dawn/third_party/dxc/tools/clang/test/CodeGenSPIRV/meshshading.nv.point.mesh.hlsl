// RUN: %dxc -T ms_6_5 -E main -fcgl  %s -spirv | FileCheck %s
// CHECK:  OpCapability MeshShadingNV
// CHECK:  OpExtension "SPV_NV_mesh_shader"
// CHECK:  OpEntryPoint MeshNV %main "main" %gl_GlobalInvocationID %gl_Position [[primind:%[0-9]+]] [[primcount:%[0-9]+]]
// CHECK:  OpExecutionMode %main LocalSize 128 1 1
// CHECK:  OpExecutionMode %main OutputPoints
// CHECK:  OpExecutionMode %main OutputVertices 256
// CHECK:  OpExecutionMode %main OutputPrimitivesEXT 256

// CHECK:  OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
// CHECK:  OpDecorate %gl_Position BuiltIn Position
// CHECK:  OpDecorate [[primind]] BuiltIn PrimitiveIndicesNV
// CHECK:  OpDecorate [[primcount]] BuiltIn PrimitiveCountNV

// CHECK:  %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
// CHECK:  %gl_Position = OpVariable %_ptr_Output__arr_v4float_uint_256 Output
// CHECK:  [[primind]] = OpVariable %_ptr_Output__arr_uint_uint_256 Output
// CHECK:  [[primcount]] = OpVariable %_ptr_Output_uint Output

struct MeshPerVertex {
    float4 position : SV_Position;
};

#define NUM_THREADS 128
#define MAX_VERT 256
#define MAX_PRIM 256

[outputtopology("point")]
[numthreads(NUM_THREADS, 1, 1)]
void main(out vertices MeshPerVertex verts[MAX_VERT],
          out indices uint primitiveInd[MAX_PRIM],
          in uint tid : SV_DispatchThreadID)
{

// CHECK:  OpStore [[primcount]] %uint_256
    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);

// CHECK:  OpLoad %uint %tid
// CHECK:  OpAccessChain %_ptr_Output_v4float %gl_Position {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    verts[tid].position = float4(4.0,5.0,6.0,7.0);

// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] %int_6
// CHECK:  OpStore {{%[0-9]+}} %uint_2
    primitiveInd[6] = 2;
}

