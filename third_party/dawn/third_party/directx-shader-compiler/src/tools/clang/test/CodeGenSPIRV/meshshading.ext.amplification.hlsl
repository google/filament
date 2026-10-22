// RUN: %dxc -T as_6_5 -fspv-target-env=vulkan1.1spirv1.4 -E main -fcgl  %s -spirv | FileCheck %s
// CHECK:  OpCapability MeshShadingEXT
// CHECK:  OpExtension "SPV_EXT_mesh_shader"
// CHECK:  OpEntryPoint TaskEXT %main "main" [[drawid:%[0-9]+]] %gl_LocalInvocationID %gl_WorkGroupID %gl_GlobalInvocationID %gl_LocalInvocationIndex %pld
// CHECK:  OpExecutionMode %main LocalSize 128 1 1

// CHECK:  OpDecorate [[drawid]] BuiltIn DrawIndex
// CHECK:  OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
// CHECK:  OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
// CHECK:  OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
// CHECK:  OpDecorate %gl_LocalInvocationIndex BuiltIn LocalInvocationIndex


// CHECK:  %pld = OpVariable %_ptr_TaskPayloadWorkgroupEXT_MeshPayload TaskPayloadWorkgroupEXT
// CHECK:  [[drawid]] = OpVariable %_ptr_Input_int Input
// CHECK:  %gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
// CHECK:  %gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input
// CHECK:  %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
// CHECK:  %gl_LocalInvocationIndex = OpVariable %_ptr_Input_uint Input
struct MeshPayload {
    float dummy[10];
    float4 pos;
};

groupshared MeshPayload pld;

#define NUM_THREADS 128

[numthreads(NUM_THREADS, 1, 1)]
void main(
// CHECK:  %param_var_drawId = OpVariable %_ptr_Function_int Function
// CHECK:  %param_var_gtid = OpVariable %_ptr_Function_v3uint Function
// CHECK:  %param_var_gid = OpVariable %_ptr_Function_v2uint Function
// CHECK:  %param_var_tid = OpVariable %_ptr_Function_uint Function
// CHECK:  %param_var_tig = OpVariable %_ptr_Function_uint Function
        [[vk::builtin("DrawIndex")]] in int drawId : DRAW,  // -> BuiltIn DrawIndex
        in uint3 gtid : SV_GroupThreadID,
        in uint2 gid : SV_GroupID,
        in uint tid : SV_DispatchThreadID,
        in uint tig : SV_GroupIndex)
{
// CHECK:  %drawId = OpFunctionParameter %_ptr_Function_int
// CHECK:  %gtid = OpFunctionParameter %_ptr_Function_v3uint
// CHECK:  %gid = OpFunctionParameter %_ptr_Function_v2uint
// CHECK:  %tid = OpFunctionParameter %_ptr_Function_uint
// CHECK:  %tig = OpFunctionParameter %_ptr_Function_uint
// 
// CHECK:  [[a:%[0-9]+]] = OpAccessChain %_ptr_TaskPayloadWorkgroupEXT_v4float %pld %int_1
// CHECK:  OpStore [[a]] {{%[0-9]+}}
    pld.pos = float4(gtid.x, gid.y, tid, tig);

// CHECK:  OpControlBarrier %uint_2 %uint_2 %uint_264
// CHECK:  [[e:%[0-9]+]] = OpLoad %MeshPayload %pld
// CHECK:  [[h:%[0-9]+]] = OpLoad %int %drawId
// CHECK:  [[i:%[0-9]+]] = OpBitcast %uint [[h]]
// CHECK:  [[j:%[0-9]+]] = OpLoad %int %drawId
// CHECK:  [[k:%[0-9]+]] = OpBitcast %uint [[j]]
// CHECK:  OpEmitMeshTasksEXT %uint_128 [[i]] [[k]]
   DispatchMesh(NUM_THREADS, drawId, drawId, pld);
}
