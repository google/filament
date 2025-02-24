// RUN: %dxc -T as_6_5 -E main -fspv-target-env=vulkan1.2 -fcgl -spirv %s | FileCheck %s

// CHECK:  OpCapability MeshShadingEXT
// CHECK:  OpExtension "SPV_EXT_mesh_shader"
// CHECK:  OpEntryPoint TaskEXT %main "main"

struct SubMesh {
    uint vertexCount;
    uint vertexOffset;
    uint primitiveCount;
    uint indexOffset;
    float4 boundingBox[8];
};

struct Mesh {
    uint firstSubmesh;
    uint submeshCount;
    uint dummy[2];
};

struct UserVertex {
    float3 position;
    float2 texcoord;
    float3 color;
};

[[vk::binding(0, 0)]]
RWStructuredBuffer<UserVertex> userVertices;

[[vk::binding(1, 0)]]
RWStructuredBuffer<uint> userIndices;

[[vk::binding(2, 0)]]
RWStructuredBuffer<Mesh> meshes;

[[vk::binding(3, 0)]]
RWStructuredBuffer<SubMesh> submeshes;

[[vk::binding(4, 0)]]
cbuffer UBO {
    row_major float4x4 mvp;
}

groupshared uint passedSubmeshes;
struct SubMeshes {
    uint submeshID[256] : SUBMESH;
};
groupshared SubMeshes sharedSubMeshes;

// CHECK:  %_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
// CHECK:  %SubMesh_0 = OpTypeStruct %uint %uint %uint %uint %_arr_v4float_uint_8_0
// CHECK:  %_ptr_Function_SubMesh_0 = OpTypePointer Function %SubMesh_0
// CHECK:  [[funcType:%[0-9]+]] = OpTypeFunction %bool %_ptr_Function_SubMesh_0

bool TestSubmesh(SubMesh submesh) {
    uint clip = 0x0U;

    for (uint bbv = 0U ; bbv < 8U; bbv++) {
        float4 pos= mul(mvp, submesh.boundingBox[bbv]);
        if (pos.x <= pos.w) clip |= 0x1U;
        if (pos.y <= 0.3333 * pos.w) clip |= 0x2U;
        if (pos.z <= pos.w) clip |= 0x4U;
        if (pos.x >= -pos.w) clip |= 0x8U;
        if (pos.y >= -pos.w) clip |= 0x10U;
        if (pos.z >= -pos.w) clip |= 0x20U;
    }
    return (clip == 0x3FU);
}

[numthreads(32, 1, 1)]
void main(
            in uint tid : SV_GroupThreadID,
            in uint mid : SV_GroupID
         )
{
    uint firstSubmesh = meshes[mid].firstSubmesh;
    uint submeshCount = meshes[mid].submeshCount;
    passedSubmeshes = 0U;
    GroupMemoryBarrier();
    for (uint i = 0U; i < submeshCount; i += 32U) {
        uint smid = firstSubmesh + i + tid;
        if (smid >= firstSubmesh + submeshCount) continue;

// CHECK:  %submesh = OpVariable %_ptr_Function_SubMesh_0 Function
// CHECK:  %passed = OpVariable %_ptr_Function_bool Function
// CHECK:  %param_var_submesh = OpVariable %_ptr_Function_SubMesh_0 Function
        SubMesh submesh = submeshes[smid];
        bool passed = true;

// CHECK:  [[submeshValue:%[0-9]+]] = OpLoad %SubMesh_0 %submesh
// CHECK:  OpStore %param_var_submesh [[submeshValue]]
// CHECK:  [[rv:%[0-9]+]] = OpFunctionCall %bool %TestSubmesh %param_var_submesh
// CHECK:  [[cond:%[0-9]+]] = OpLogicalNot %bool [[rv]]
// CHECK:  OpSelectionMerge %if_merge_0 None
// CHECK:  OpBranchConditional [[cond]] %if_true_0 %if_merge_0
// CHECK:  %if_true_0 = OpLabel
// CHECK:  OpStore %passed %false
// CHECK:  OpBranch %if_merge_0
// CHECK:  %if_merge_0 = OpLabel
        if (!TestSubmesh(submesh)) passed = false;

        if (passed) {
            uint ballot = WaveActiveBallot(passed).x;
            uint laneMaskLT = (1 << WaveGetLaneIndex()) - 1;
            uint lowerThreads = ballot & laneMaskLT;
            uint slot = passedSubmeshes + WavePrefixCountBits(passed);
            sharedSubMeshes.submeshID[slot] = smid;
            if (lowerThreads == 0U) {
                passedSubmeshes += WaveActiveCountBits(passed);
            }
        }
        GroupMemoryBarrier();
    }
    DispatchMesh(passedSubmeshes, 1, 1, sharedSubMeshes);
}

/* bool TestSubmesh(SubMesh submesh) { ... } */

// CHECK:  %TestSubmesh = OpFunction %bool None [[funcType]]
// CHECK:  %submesh_0 = OpFunctionParameter %_ptr_Function_SubMesh_0

// CHECK:  %bb_entry_0 = OpLabel

// CHECK:  %clip = OpVariable %_ptr_Function_uint Function
// CHECK:  %bbv = OpVariable %_ptr_Function_uint Function
// CHECK:  %pos = OpVariable %_ptr_Function_v4float Function

// CHECK:  %for_check_0 = OpLabel
// CHECK:  %for_body_0 = OpLabel
// CHECK:  %for_merge_0 = OpLabel

// CHECK:  [[clipValue:%[0-9]+]] = OpLoad %uint %clip
// CHECK:  [[retValue:%[0-9]+]] = OpIEqual %bool [[clipValue]] %uint_63
// CHECK:  OpReturnValue [[retValue]]
// CHECK:  OpFunctionEnd
