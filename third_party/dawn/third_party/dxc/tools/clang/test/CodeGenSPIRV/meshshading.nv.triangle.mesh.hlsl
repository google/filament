// RUN: %dxc -T ms_6_5 -E main -fcgl  %s -spirv | FileCheck %s
// CHECK:  OpCapability MeshShadingNV
// CHECK:  OpExtension "SPV_NV_mesh_shader"
// CHECK:  OpEntryPoint MeshNV %main "main" %gl_ClipDistance %gl_CullDistance %in_var_dummy %in_var_pos [[drawid:%[0-9]+]] %gl_LocalInvocationID %gl_WorkGroupID %gl_GlobalInvocationID %gl_LocalInvocationIndex %gl_Position %gl_PointSize %out_var_USER %out_var_USER_ARR %out_var_USER_MAT [[primind:%[0-9]+]] %gl_PrimitiveID %gl_Layer %gl_ViewportIndex [[vmask:%[0-9]+]] %out_var_PRIM_USER %out_var_PRIM_USER_ARR [[primcount:%[0-9]+]]
// CHECK:  OpExecutionMode %main LocalSize 128 1 1
// CHECK:  OpExecutionMode %main OutputTrianglesEXT
// CHECK:  OpExecutionMode %main OutputVertices 64
// CHECK:  OpExecutionMode %main OutputPrimitivesEXT 81

// CHECK:  OpDecorate %gl_ClipDistance BuiltIn ClipDistance
// CHECK:  OpDecorate %gl_CullDistance BuiltIn CullDistance
// CHECK:  OpDecorate %in_var_dummy PerTaskNV
// CHECK:  OpDecorate %in_var_dummy Offset 0
// CHECK:  OpDecorate %in_var_pos PerTaskNV
// CHECK:  OpDecorate %in_var_pos Offset 48
// CHECK:  OpDecorate [[drawid]] BuiltIn DrawIndex
// CHECK:  OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
// CHECK:  OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
// CHECK:  OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
// CHECK:  OpDecorate %gl_LocalInvocationIndex BuiltIn LocalInvocationIndex
// CHECK:  OpDecorate %gl_Position BuiltIn Position
// CHECK:  OpDecorate %gl_PointSize BuiltIn PointSize
// CHECK:  OpDecorate [[primind]] BuiltIn PrimitiveIndicesNV
// CHECK:  OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
// CHECK:  OpDecorate %gl_PrimitiveID PerPrimitiveEXT
// CHECK:  OpDecorate %gl_Layer BuiltIn Layer
// CHECK:  OpDecorate %gl_Layer PerPrimitiveEXT
// CHECK:  OpDecorate %gl_ViewportIndex BuiltIn ViewportIndex
// CHECK:  OpDecorate %gl_ViewportIndex PerPrimitiveEXT
// CHECK:  OpDecorate [[vmask]] BuiltIn ViewportMaskNV
// CHECK:  OpDecorate [[vmask]] PerPrimitiveEXT
// CHECK:  OpDecorate %out_var_PRIM_USER PerPrimitiveEXT
// CHECK:  OpDecorate %out_var_PRIM_USER_ARR PerPrimitiveEXT
// CHECK:  OpDecorate [[primcount]] BuiltIn PrimitiveCountNV
// CHECK:  OpDecorate %out_var_USER Location 0
// CHECK:  OpDecorate %out_var_USER_ARR Location 1
// CHECK:  OpDecorate %out_var_USER_MAT Location 3
// CHECK:  OpDecorate %out_var_PRIM_USER Location 7
// CHECK:  OpDecorate %out_var_PRIM_USER_ARR Location 8

// CHECK:  %gl_ClipDistance = OpVariable %_ptr_Output__arr__arr_float_uint_5_uint_64 Output
// CHECK:  %gl_CullDistance = OpVariable %_ptr_Output__arr__arr_float_uint_3_uint_64 Output
// CHECK:  %in_var_dummy = OpVariable %_ptr_Input__arr_float_uint_10 Input
// CHECK:  %in_var_pos = OpVariable %_ptr_Input_v4float Input
// CHECK:  %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
// CHECK:  %gl_LocalInvocationIndex = OpVariable %_ptr_Input_uint Input
// CHECK:  %gl_Position = OpVariable %_ptr_Output__arr_v4float_uint_64 Output
// CHECK:  %gl_PointSize = OpVariable %_ptr_Output__arr_float_uint_64 Output
// CHECK:  %out_var_USER = OpVariable %_ptr_Output__arr_v2float_uint_64 Output
// CHECK:  %out_var_USER_ARR = OpVariable %_ptr_Output__arr__arr_v4float_uint_2_uint_64 Output
// CHECK:  %out_var_USER_MAT = OpVariable %_ptr_Output__arr_mat4v4float_uint_64 Output
// CHECK:  [[primind]] = OpVariable %_ptr_Output__arr_uint_uint_243 Output
// CHECK:  %gl_PrimitiveID = OpVariable %_ptr_Output__arr_int_uint_81 Output
// CHECK:  %gl_Layer = OpVariable %_ptr_Output__arr_int_uint_81 Output
// CHECK:  %gl_ViewportIndex = OpVariable %_ptr_Output__arr_int_uint_81 Output
// CHECK:  [[vmask]] = OpVariable %_ptr_Output__arr__arr_int_uint_1_uint_81 Output
// CHECK:  %out_var_PRIM_USER = OpVariable %_ptr_Output__arr_v3float_uint_81 Output
// CHECK:  %out_var_PRIM_USER_ARR = OpVariable %_ptr_Output__arr__arr_v4float_uint_2_uint_81 Output
// CHECK:  [[primcount]] = OpVariable %_ptr_Output_uint Output

struct MeshPerVertex {
    float4 position : SV_Position;                          // -> BuiltIn Position
    [[vk::builtin("PointSize")]] float psize : PSIZE;       // -> BuiltIn PointSize
    float3 clipdis4 : SV_ClipDistance4;                     // -> BuiltIn ClipDistance
    float  culldis5 : SV_CullDistance5;                     // -> BuiltIn CullDistance
    float2 clipdis3 : SV_ClipDistance3;                     // -> BuiltIn ClipDistance
    float2 culldis6 : SV_CullDistance6;                     // -> BuiltIn CullDistance
    float2 userVertAttr : USER;
    float4 userVertAttrArr[2] : USER_ARR;
    float4x4 userVertAttrMat : USER_MAT;
};

struct MeshPerPrimitive {
    int primId : SV_PrimitiveID;                            // -> Builtin PrimitiveId
    int layer  : SV_RenderTargetArrayIndex;                 // -> Builtin Layer
    int vpIdx  : SV_ViewportArrayIndex;                     // -> Builtin ViewportIndex
    [[vk::builtin("ViewportMaskNV")]] int vmask[1] : VMASK; // -> BuiltIn ViewportMaskNV
    float3 userPrimAttr : PRIM_USER;
    float4 userPrimAttrArr[2] : PRIM_USER_ARR;
};

struct MeshPayload {
    float dummy[10];
    float4 pos;
};

#define MAX_VERT 64
#define MAX_PRIM 81
#define NUM_THREADS 128

[outputtopology("triangle")]
[numthreads(NUM_THREADS, 1, 1)]
void main(
// CHECK:  %param_var_verts = OpVariable %_ptr_Function__arr_MeshPerVertex_uint_64 Function
// CHECK:  %param_var_primitiveInd = OpVariable %_ptr_Function__arr_v3uint_uint_81 Function
// CHECK:  %param_var_prims = OpVariable %_ptr_Function__arr_MeshPerPrimitive_uint_81 Function
// CHECK:  %param_var_pld = OpVariable %_ptr_Function_MeshPayload Function
// CHECK:  %param_var_drawId = OpVariable %_ptr_Function_int Function
// CHECK:  %param_var_gtid = OpVariable %_ptr_Function_v3uint Function
// CHECK:  %param_var_gid = OpVariable %_ptr_Function_v2uint Function
// CHECK:  %param_var_tid = OpVariable %_ptr_Function_uint Function
// CHECK:  %param_var_tig = OpVariable %_ptr_Function_uint Function
        out vertices MeshPerVertex verts[MAX_VERT],
        out indices uint3 primitiveInd[MAX_PRIM],
        out primitives MeshPerPrimitive prims[MAX_PRIM],
        in payload MeshPayload pld,
        [[vk::builtin("DrawIndex")]] in int drawId : DRAW,  // -> BuiltIn DrawIndex
        in uint3 gtid : SV_GroupThreadID,
        in uint2 gid : SV_GroupID,
        in uint tid : SV_DispatchThreadID,
        in uint tig : SV_GroupIndex)
{
// CHECK:  OpStore [[primcount]] %uint_81
    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);

    // Directly assign to per-vertex attribute object.

// CHECK:  OpAccessChain %_ptr_Output_float %gl_Position {{%[0-9]+}} %uint_0
// CHECK:  OpStore {{%[0-9]+}} %float_11
    verts[tid].position.x = 11.0;
// CHECK:  OpAccessChain %_ptr_Output_float %gl_Position {{%[0-9]+}} %uint_1
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpAccessChain %_ptr_Output_float %gl_Position {{%[0-9]+}} %uint_3
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    verts[tid].position.yw = float2(12.0,14.0);
// CHECK:  OpAccessChain %_ptr_Output_float %gl_Position {{%[0-9]+}} %uint_2
// CHECK:  OpStore {{%[0-9]+}} %float_13
    verts[tid].position[2] = 13.0;
// CHECK:  OpAccessChain %_ptr_Output_float %gl_PointSize {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %float_50
    verts[tid].psize = 50.0;
// CHECK:  OpIAdd %uint %uint_1 %uint_2
// CHECK:  OpAccessChain %_ptr_Output_float %gl_ClipDistance {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpIAdd %uint %uint_0 %uint_2
// CHECK:  OpAccessChain %_ptr_Output_float %gl_ClipDistance {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpIAdd %uint %uint_2 %uint_2
// CHECK:  OpAccessChain %_ptr_Output_float %gl_ClipDistance {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    verts[tid].clipdis4.yxz = float3(0.0,1.0,2.0);
// CHECK:  OpIAdd %uint %uint_0 %uint_2
// CHECK:  OpAccessChain %_ptr_Output_float %gl_ClipDistance {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %float_10
    verts[tid].clipdis4[0] = 10.0;
// CHECK:  OpAccessChain %_ptr_Output_float %gl_CullDistance {{%[0-9]+}} %uint_0
// CHECK:  OpStore {{%[0-9]+}} %float_5
    verts[tid].culldis5 = 5.0;
// CHECK:  OpIAdd %uint %uint_0 %uint_0
// CHECK:  OpAccessChain %_ptr_Output_float %gl_ClipDistance {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpIAdd %uint %uint_0 %uint_1
// CHECK:  OpAccessChain %_ptr_Output_float %gl_ClipDistance {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    verts[tid].clipdis3 = float2(11.0,12.0);
// CHECK:  OpIAdd %uint %uint_0 %uint_1
// CHECK:  OpAccessChain %_ptr_Output_float %gl_CullDistance {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %float_13
    verts[tid].culldis6[0] = 13.0;
// CHECK:  OpIAdd %uint %uint_1 %uint_1
// CHECK:  OpAccessChain %_ptr_Output_float %gl_CullDistance {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %float_14
    verts[tid].culldis6.y = 14.0;
// CHECK:  OpAccessChain %_ptr_Output_v2float %out_var_USER {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    verts[tid].userVertAttr = float2(9.0, 10.0);
// CHECK:  OpAccessChain %_ptr_Output_v4float %out_var_USER_ARR {{%[0-9]+}} %int_0
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    verts[tid].userVertAttrArr[0] = float4(17.0, 18.0, 19.0, 20.0);
// CHECK:  OpAccessChain %_ptr_Output_v4float %out_var_USER_ARR {{%[0-9]+}} %int_1
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    verts[tid].userVertAttrArr[1] = float4(27.0, 28.0, 29.0, 30.0);
// CHECK:  OpAccessChain %_ptr_Output_v4float %out_var_USER_MAT {{%[0-9]+}} %uint_3
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    verts[tid].userVertAttrMat[3] = float4(7.0, 8.0, 9.0, 10.0);

    // Indirectly assign to per-vertex attribute object.
    MeshPerVertex vert;
    vert.position = pld.pos;
    vert.psize = 50.0;
    vert.clipdis4.yxz = float3(0.0,1.0,2.0);
    vert.clipdis4[0] = 10.0;
    vert.culldis5 = 5.0;
    vert.clipdis3 = float2(11.0,12.0);
    vert.culldis6[0] = 13.0;
    vert.culldis6.y = 14.0;
    vert.userVertAttr = float2(9.0, 10.0);
    vert.userVertAttrArr[0] = float4(17.0, 18.0, 19.0, 20.0);
    vert.userVertAttrArr[1] = float4(27.0, 28.0, 29.0, 30.0);
    vert.userVertAttrMat[3] = float4(7.0, 8.0, 9.0, 10.0);
    verts[tid+1] = vert;

    // Directly assign to per-vertex attribute object.
 
// CHECK:  OpAccessChain %_ptr_Output_int %gl_PrimitiveID {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %int_10
    prims[tig].primId = 10;
// CHECK:  OpAccessChain %_ptr_Output_int %gl_Layer {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %int_11
    prims[tig].layer = 11;
// CHECK:  OpAccessChain %_ptr_Output_int %gl_ViewportIndex {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %int_12
    prims[tig].vpIdx = 12;
// CHECK:  OpAccessChain %_ptr_Output_int [[vmask]] {{%[0-9]+}} %int_0
// CHECK:  OpStore {{%[0-9]+}} %int_32
    prims[tig].vmask[0] = 32;
// CHECK:  OpAccessChain %_ptr_Output_v4float %out_var_PRIM_USER_ARR {{%[0-9]+}} %int_0
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    prims[tig].userPrimAttrArr[0] = float4(4.0,5.0,6.0,7.0);
// CHECK:  OpAccessChain %_ptr_Output_v4float %out_var_PRIM_USER_ARR {{%[0-9]+}} %int_1
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    prims[tig].userPrimAttrArr[1] = float4(8.0,9.0,10.0,11.0);
// CHECK:  OpAccessChain %_ptr_Output_v3float %out_var_PRIM_USER {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    prims[tig].userPrimAttr = float3(14.0,15.0,16.0);

    // Indirectly assign to per-vertex attribute object.
    MeshPerPrimitive prim;
    prim.primId = 10;
    prim.layer = 11;
    prim.vpIdx = 12;
    prim.vmask[0] = 32;
    prim.userPrimAttrArr[0] = float4(4.0,5.0,6.0,7.0);
    prim.userPrimAttrArr[1] = float4(8.0,9.0,10.0,11.0);
    prim.userPrimAttr = float3(14.0,15.0,16.0);
    prims[tig+1] = prim;
 
    // Assign primitive indices.

// CHECK:  OpIMul %uint %uint_4 %uint_3
// CHECK:  OpIAdd %uint {{%[0-9]+}} %uint_0
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %uint_1
    primitiveInd[4].x = 1;
// CHECK:  OpCompositeExtract %uint {{%[0-9]+}} 0
// CHECK:  OpIMul %uint %uint_4 %uint_3
// CHECK:  OpIAdd %uint {{%[0-9]+}} %uint_1
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpCompositeExtract %uint {{%[0-9]+}} 1
// CHECK:  OpIMul %uint %uint_4 %uint_3
// CHECK:  OpIAdd %uint {{%[0-9]+}} %uint_2
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    primitiveInd[4].yz = uint2(2,3);
// CHECK:  OpIMul %uint %uint_2 %uint_3
// CHECK:  OpIAdd %uint {{%[0-9]+}} %uint_1
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %uint_2
    primitiveInd[2].y = 2;
// CHECK:  OpIMul %uint %uint_2 %uint_3
// CHECK:  OpIAdd %uint {{%[0-9]+}} %uint_2
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%[0-9]+}}
// CHECK:  OpStore {{%[0-9]+}} %uint_1
    primitiveInd[2][2] = 1;
// CHECK:  OpLoad %uint %tid
// CHECK:  OpIMul %uint {{%[0-9]+}} %uint_3
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%[0-9]+}}
// CHECK:  OpCompositeExtract %uint {{%[0-9]+}} 0
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpIAdd %uint {{%[0-9]+}} %uint_1
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%[0-9]+}}
// CHECK:  OpCompositeExtract %uint {{%[0-9]+}} 1
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
// CHECK:  OpIAdd %uint {{%[0-9]+}} %uint_2
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%[0-9]+}}
// CHECK:  OpCompositeExtract %uint {{%[0-9]+}} 2
// CHECK:  OpStore {{%[0-9]+}} {{%[0-9]+}}
    primitiveInd[tid] = uint3(11,12,13);
}
