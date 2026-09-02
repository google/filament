// RUN: %dxc -T ms_6_5 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:  OpCapability MeshShadingNV
// CHECK:  OpExtension "SPV_NV_mesh_shader"
// CHECK:  OpEntryPoint MeshNV %main "main"

// CHECK:  OpName %UserVertex "UserVertex"
struct UserVertex {
// CHECK:  OpMemberName %UserVertex 0 "position"
// CHECK:  OpMemberName %UserVertex 1 "texcoord"
// CHECK:  OpMemberName %UserVertex 2 "color"
    float3 position;
    float2 texcoord;
    float3 color;
};

// CHECK:  OpName %Mesh "Mesh"
struct Mesh {
// CHECK:  OpMemberName %Mesh 0 "firstSubmesh"
// CHECK:  OpMemberName %Mesh 1 "submeshCount"
// CHECK:  OpMemberName %Mesh 2 "dummy"
    uint firstSubmesh;
    uint submeshCount;
    uint dummy[2];
};

// CHECK:  OpName %SubMesh "SubMesh"
struct SubMesh {
// CHECK:  OpMemberName %SubMesh 0 "vertexCount"
// CHECK:  OpMemberName %SubMesh 1 "vertexOffset"
// CHECK:  OpMemberName %SubMesh 2 "primitiveCount"
// CHECK:  OpMemberName %SubMesh 3 "indexOffset"
// CHECK:  OpMemberName %SubMesh 4 "boundingBox"
    uint vertexCount;
    uint vertexOffset;
    uint primitiveCount;
    uint indexOffset;
    float4 boundingBox[8];
};

// CHECK:  OpDecorate %userVertices DescriptorSet 0
// CHECK:  OpDecorate %userVertices Binding 0
// CHECK:  OpDecorate %userIndices DescriptorSet 0
// CHECK:  OpDecorate %userIndices Binding 1
// CHECK:  OpDecorate %meshes DescriptorSet 0
// CHECK:  OpDecorate %meshes Binding 2
// CHECK:  OpDecorate %submeshes DescriptorSet 0
// CHECK:  OpDecorate %submeshes Binding 3
// CHECK:  OpDecorate %UBO DescriptorSet 0
// CHECK:  OpDecorate %UBO Binding 4

// CHECK:  OpMemberDecorate %UserVertex 0 Offset 0
// CHECK:  OpMemberDecorate %UserVertex 1 Offset 16
// CHECK:  OpMemberDecorate %UserVertex 2 Offset 32
// CHECK:  OpDecorate %_runtimearr_UserVertex ArrayStride 48
// CHECK:  OpMemberDecorate %type_RWStructuredBuffer_UserVertex 0 Offset 0
// CHECK:  OpDecorate %type_RWStructuredBuffer_UserVertex BufferBlock

// CHECK:  OpDecorate %_runtimearr_uint ArrayStride 4
// CHECK:  OpMemberDecorate %type_RWStructuredBuffer_uint 0 Offset 0
// CHECK:  OpDecorate %type_RWStructuredBuffer_uint BufferBlock

// CHECK:  OpMemberDecorate %Mesh 0 Offset 0
// CHECK:  OpMemberDecorate %Mesh 1 Offset 4
// CHECK:  OpMemberDecorate %Mesh 2 Offset 8
// CHECK:  OpDecorate %_runtimearr_Mesh ArrayStride 16
// CHECK:  OpMemberDecorate %type_RWStructuredBuffer_Mesh 0 Offset 0
// CHECK:  OpDecorate %type_RWStructuredBuffer_Mesh BufferBlock

// CHECK:  OpMemberDecorate %SubMesh 0 Offset 0
// CHECK:  OpMemberDecorate %SubMesh 1 Offset 4
// CHECK:  OpMemberDecorate %SubMesh 2 Offset 8
// CHECK:  OpMemberDecorate %SubMesh 3 Offset 12
// CHECK:  OpMemberDecorate %SubMesh 4 Offset 16
// CHECK:  OpDecorate %_runtimearr_SubMesh ArrayStride 144
// CHECK:  OpMemberDecorate %type_RWStructuredBuffer_SubMesh 0 Offset 0
// CHECK:  OpDecorate %type_RWStructuredBuffer_SubMesh BufferBlock

// CHECK:  OpMemberDecorate %type_UBO 0 Offset 0
// CHECK:  OpMemberDecorate %type_UBO 0 MatrixStride 16
// CHECK:  OpMemberDecorate %type_UBO 0 ColMajor
// CHECK:  OpDecorate %type_UBO Block

// CHECK:  %UserVertex = OpTypeStruct %v3float %v2float %v3float
// CHECK:  %_runtimearr_UserVertex = OpTypeRuntimeArray %UserVertex
// CHECK:  %type_RWStructuredBuffer_UserVertex = OpTypeStruct %_runtimearr_UserVertex
// CHECK:  %_ptr_Uniform_type_RWStructuredBuffer_UserVertex = OpTypePointer Uniform %type_RWStructuredBuffer_UserVertex
[[vk::binding(0, 0)]]
RWStructuredBuffer<UserVertex> userVertices;

// CHECK:  %_runtimearr_uint = OpTypeRuntimeArray %uint
// CHECK:  %type_RWStructuredBuffer_uint = OpTypeStruct %_runtimearr_uint
// CHECK:  %_ptr_Uniform_type_RWStructuredBuffer_uint = OpTypePointer Uniform %type_RWStructuredBuffer_uint
[[vk::binding(1, 0)]]
RWStructuredBuffer<uint> userIndices;

// CHECK:  %_arr_uint_uint_2 = OpTypeArray %uint %uint_2
// CHECK:  %Mesh = OpTypeStruct %uint %uint %_arr_uint_uint_2
// CHECK:  %_runtimearr_Mesh = OpTypeRuntimeArray %Mesh
// CHECK:  %type_RWStructuredBuffer_Mesh = OpTypeStruct %_runtimearr_Mesh
// CHECK:  %_ptr_Uniform_type_RWStructuredBuffer_Mesh = OpTypePointer Uniform %type_RWStructuredBuffer_Mesh
[[vk::binding(2, 0)]]
RWStructuredBuffer<Mesh> meshes;

// CHECK:  %uint_8 = OpConstant %uint 8
// CHECK:  %v4float = OpTypeVector %float 4
// CHECK:  %_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
// CHECK:  %SubMesh = OpTypeStruct %uint %uint %uint %uint %_arr_v4float_uint_8
// CHECK:  %_runtimearr_SubMesh = OpTypeRuntimeArray %SubMesh
// CHECK:  %type_RWStructuredBuffer_SubMesh = OpTypeStruct %_runtimearr_SubMesh
// CHECK:  %_ptr_Uniform_type_RWStructuredBuffer_SubMesh = OpTypePointer Uniform %type_RWStructuredBuffer_SubMesh
[[vk::binding(3, 0)]]
RWStructuredBuffer<SubMesh> submeshes;

// CHECK:  %mat4v4float = OpTypeMatrix %v4float 4
// CHECK:  %type_UBO = OpTypeStruct %mat4v4float
// CHECK:  %_ptr_Uniform_type_UBO = OpTypePointer Uniform %type_UBO
[[vk::binding(4, 0)]]
cbuffer UBO {
    row_major float4x4 mvp;
}

struct PerVertex {
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
    float3 color : COLOR;
};

struct PerPrimitive {
    float4 primcolor : PCOLOR;
};

struct SubMeshes {
    uint submeshID[256] : SUBMESH;
};

static const uint vertsPerPrim = 3U;

// CHECK:  %userVertices = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_UserVertex Uniform
// CHECK:  %userIndices = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_uint Uniform
// CHECK:  %meshes = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_Mesh Uniform
// CHECK:  %submeshes = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_SubMesh Uniform
// CHECK:  %UBO = OpVariable %_ptr_Uniform_type_UBO Uniform

[outputtopology("triangle")]
[numthreads(32, 1, 1)]
void main(
            out indices uint3 primIndices[128],
            out vertices PerVertex verts[128],
            out primitives PerPrimitive prims[128],
            in payload SubMeshes taskmem,
            in uint gid : SV_GroupID,
            in uint tid : SV_GroupThreadID
         )
{
    uint task = taskmem.submeshID[gid];
// CHECK:  %submesh = OpVariable %_ptr_Function_SubMesh_0 Function
// CHECK:  OpAccessChain %_ptr_Uniform_SubMesh %submeshes %int_0 [[task:%[0-9]+]]
// CHECK:  OpStore %submesh [[submeshVal:%[0-9]+]]
    SubMesh submesh = submeshes[task];
// CHECK:  OpAccessChain %_ptr_Function_uint %submesh %int_0
    uint numPackedVertices = submesh.vertexCount;
// CHECK:  OpAccessChain %_ptr_Function_uint %submesh %int_2
    uint numPackedPrimitives = submesh.primitiveCount;

    SetMeshOutputCounts(numPackedVertices, numPackedPrimitives);

    for (uint i = 0U; i < numPackedVertices; i += 32U) {
        uint vid = i + tid;
// CHECK:  OpAccessChain %_ptr_Function_uint %submesh %int_1
        uint svid = vid + submesh.vertexOffset;
        if (vid >= numPackedVertices) continue;
// CHECK:  OpAccessChain %_ptr_Uniform_v2float %userVertices %int_0 [[svid_1:%[0-9]+]] %int_1
        verts[vid].texcoord = userVertices[svid].texcoord;
// CHECK:  OpAccessChain %_ptr_Uniform_v3float %userVertices %int_0 [[svid_2:%[0-9]+]] %int_2
        verts[vid].color = userVertices[svid].color;
// CHECK:  OpAccessChain %_ptr_Uniform_v3float %userVertices %int_0 [[svid_0:%[0-9]+]] %int_0
        float3 position = userVertices[svid].position;
// CHECK:  OpAccessChain %_ptr_Uniform_mat4v4float %UBO %int_0 
        verts[vid].position = mul(mvp, float4(position, 1.0));
    }

    GroupMemoryBarrier();

    for (uint j = 0U; j < numPackedPrimitives; j += 32U) {
        uint pid = j + tid;
        uint didxoff = vertsPerPrim * pid;
// CHECK:  OpAccessChain %_ptr_Function_uint %submesh %int_3
        uint sidxoff = submesh.indexOffset + didxoff;
        if (pid >= numPackedPrimitives) continue;
// CHECK:  OpAccessChain %_ptr_Uniform_uint %userIndices %int_0 [[sidxoff_0:%[0-9]+]]
// CHECK:  OpAccessChain %_ptr_Uniform_uint %userIndices %int_0 [[sidxoff_1:%[0-9]+]]
// CHECK:  OpAccessChain %_ptr_Uniform_uint %userIndices %int_0 [[sidxoff_2:%[0-9]+]]
        primIndices[pid] = uint3(userIndices[sidxoff], userIndices[sidxoff+1], userIndices[sidxoff+2]);
// CHECK:  OpAccessChain %_ptr_Function_uint %submesh %int_1
// CHECK:  OpAccessChain %_ptr_Uniform_uint %userIndices %int_0 [[ind:%[0-9]+]]
        uint providx = submesh.vertexOffset + userIndices[sidxoff + vertsPerPrim - 1U];
// CHECK:  OpAccessChain %_ptr_Uniform_v3float %userVertices %int_0 [[providx:%[0-9]+]] %int_2
        prims[pid].primcolor = float4(userVertices[providx].color, 1.0);
    }
}

