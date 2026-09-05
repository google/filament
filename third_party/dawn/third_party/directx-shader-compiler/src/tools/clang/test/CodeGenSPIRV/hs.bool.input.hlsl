// RUN: %dxc -T hs_6_0 -E Hull -fcgl  %s -spirv | FileCheck %s

struct ControlPoint
{
    bool b : MY_BOOL;
};

struct HullPatchOut {
    float edge [3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

// Check that the wrapper function correctly copies `v` as a parameter to Hull.
// CHECK: [[ld:%[0-9]+]] = OpLoad %_arr_uint_uint_3 %in_var_MY_BOOL
// CHECK: [[element1:%[0-9]+]] = OpCompositeExtract %uint [[ld:%[0-9]+]] 0
// CHECK: [[bool1:%[0-9]+]] = OpINotEqual %bool [[element1]] %uint_0
// CHECK: [[element2:%[0-9]+]] = OpCompositeExtract %uint [[ld:%[0-9]+]] 1
// CHECK: [[bool2:%[0-9]+]] = OpINotEqual %bool [[element2]] %uint_0
// CHECK: [[element3:%[0-9]+]] = OpCompositeExtract %uint [[ld:%[0-9]+]] 2
// CHECK: [[bool3:%[0-9]+]] = OpINotEqual %bool [[element3]] %uint_0
// CHECK: [[bool_array:%[0-9]+]] = OpCompositeConstruct %_arr_bool_uint_3 [[bool1]] [[bool2]] [[bool3]]
// CHECK: [[element1:%[0-9]+]] = OpCompositeExtract %bool [[bool_array]] 0
// CHECK: [[cp1:%[0-9]+]] = OpCompositeConstruct %ControlPoint [[element1]]
// CHECK: [[element2:%[0-9]+]] = OpCompositeExtract %bool [[bool_array]] 1
// CHECK: [[cp2:%[0-9]+]] = OpCompositeConstruct %ControlPoint [[element2]]
// CHECK: [[element3:%[0-9]+]] = OpCompositeExtract %bool [[bool_array]] 2
// CHECK: [[cp3:%[0-9]+]] = OpCompositeConstruct %ControlPoint [[element3]]
// CHECK: [[v:%[0-9]+]] = OpCompositeConstruct %_arr_ControlPoint_uint_3 [[cp1]] [[cp2]] [[cp3]]
// CHECK: OpStore %param_var_v [[v]]
// CHECK: [[ret:%[0-9]+]] = OpFunctionCall %ControlPoint %src_Hull %param_var_v %param_var_id

// Check that the return value is correctly copied to the output variable.
// CHECK: [[ret_bool:%[0-9]+]] = OpCompositeExtract %bool [[ret]] 0
// CHECK: [[ret_int:%[0-9]+]] = OpSelect %uint [[ret_bool]] %uint_1 %uint_0
// CHECK: [[out_var:%[0-9]+]] = OpAccessChain %_ptr_Output_uint %out_var_MY_BOOL
// CHECK: OpStore [[out_var]] [[ret_int]]

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HullConst")]
[outputcontrolpoints(3)]
ControlPoint Hull (InputPatch<ControlPoint,3> v, uint id : SV_OutputControlPointID) { return v[id]; }
HullPatchOut HullConst (InputPatch<ControlPoint,3> v) { return (HullPatchOut)0; }

[domain("tri")]
float4 Domain (const OutputPatch<ControlPoint,3> vi) : SV_Position { return (vi[0].b ? 1 : 0); }
