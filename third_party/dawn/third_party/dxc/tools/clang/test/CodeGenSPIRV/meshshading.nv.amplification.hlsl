// RUN: %dxc -T as_6_5 -E main -fcgl  %s -spirv | FileCheck %s
// CHECK:  OpCapability MeshShadingNV
// CHECK:  OpExtension "SPV_NV_mesh_shader"
// CHECK:  OpEntryPoint TaskNV %main "main" [[drawid:%[0-9]+]] %gl_LocalInvocationID %gl_WorkGroupID %gl_GlobalInvocationID %gl_LocalInvocationIndex %out_var_dummy %out_var_pos [[taskcount:%[0-9]+]]
// CHECK:  OpExecutionMode %main LocalSize 128 1 1

// CHECK:  OpDecorate [[drawid]] BuiltIn DrawIndex
// CHECK:  OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
// CHECK:  OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
// CHECK:  OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
// CHECK:  OpDecorate %gl_LocalInvocationIndex BuiltIn LocalInvocationIndex

struct MeshPayload {
// CHECK:  OpDecorate %out_var_dummy PerTaskNV
// CHECK:  OpDecorate %out_var_dummy Offset 0
// CHECK:  OpDecorate %out_var_pos PerTaskNV
// CHECK:  OpDecorate %out_var_pos Offset 48
    float dummy[10];
    float4 pos;
};

// CHECK:  OpDecorate [[taskcount]] BuiltIn TaskCountNV

// CHECK:  %pld = OpVariable %_ptr_Workgroup_MeshPayload Workgroup
// CHECK:  [[drawid]] = OpVariable %_ptr_Input_int Input
groupshared MeshPayload pld;

// CHECK:  %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
// CHECK:  %gl_LocalInvocationIndex = OpVariable %_ptr_Input_uint Input
// CHECK:  %out_var_dummy = OpVariable %_ptr_Output__arr_float_uint_10 Output
// CHECK:  %out_var_pos = OpVariable %_ptr_Output_v4float Output
// CHECK:  [[taskcount]] = OpVariable %_ptr_Output_uint Output

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

// CHECK:  [[b:%[0-9]+]] = OpAccessChain %_ptr_Workgroup_v4float %pld %int_1
// CHECK:  OpStore [[b]] {{%[0-9]+}}
    pld.pos = float4(gtid.x, gid.y, tid, tig);

// CHECK:  OpControlBarrier %uint_2 %uint_2 %uint_264
// CHECK:  [[c:%[0-9]+]] = OpLoad %MeshPayload %pld
// CHECK:  [[d:%[0-9]+]] = OpCompositeExtract %_arr_float_uint_10 [[c]] 0
// CHECK:  OpStore %out_var_dummy [[d]]
// CHECK:  [[e:%[0-9]+]] = OpCompositeExtract %v4float [[c]] 1
// CHECK:  OpStore %out_var_pos [[e]]
// CHECK:  [[f:%[0-9]+]] = OpLoad %int %drawId
// CHECK:  [[g:%[0-9]+]] = OpBitcast %uint [[f]]
// CHECK:  [[h:%[0-9]+]] = OpLoad %int %drawId
// CHECK:  [[i:%[0-9]+]] = OpBitcast %uint [[h]]
// CHECK:  [[j:%[0-9]+]] = OpIMul %uint [[g]] [[i]]
// CHECK:  [[k:%[0-9]+]] = OpIMul %uint %uint_128 [[j]]
// CHECK:  OpStore [[taskcount]] [[k]]
   DispatchMesh(NUM_THREADS, drawId, drawId, pld);
}
