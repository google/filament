// RUN: %dxc -T hs_6_0 -E Hull -fcgl  %s -spirv | FileCheck %s

struct ControlPoint { float4 position : POSITION; };

// CHECK: %param_var_edge = OpVariable %_ptr_Function__arr_float_uint_3 Function
// CHECK: %param_var_inside = OpVariable %_ptr_Function_float Function
// CHECK: %param_var_myFloat = OpVariable %_ptr_Function_float Function
// CHECK: OpFunctionCall %void %HullConst %param_var_edge %param_var_inside %param_var_myFloat 
// CHECK: [[edges:%[0-9]+]] = OpLoad %_arr_float_uint_3 %param_var_edge 
// CHECK: [[addr:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_0 
// CHECK: [[val:%[0-9]+]] = OpCompositeExtract %float [[arr:%[0-9]+]] 0 
// CHECK: OpStore [[addr]] [[val]]
// CHECK: [[addr:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_1 
// CHECK: [[val:%[0-9]+]] = OpCompositeExtract %float [[arr]] 1 
// CHECK: OpStore [[addr]] [[val]]
// CHECK: [[addr:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_2 
// CHECK: [[val:%[0-9]+]] = OpCompositeExtract %float [[arr]] 2 
// CHECK: OpStore [[addr]] [[val]]
// CHECK: [[val:%[0-9]+]] = OpLoad %float %param_var_inside 
// CHECK: [[addr:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %uint_0 
// CHECK: OpStore [[addr]] [[val]]
// CHECK: [[val:%[0-9]+]] = OpLoad %float %param_var_myFloat 
// CHECK: OpStore %out_var_MY_FLOAT [[val]]

void HullConst (out  float edge [3] : SV_TessFactor, out float inside : SV_InsideTessFactor, out float myFloat : MY_FLOAT)
{
    edge[0] = 2;
    edge[1] = 2;
    edge[2] = 2;
    inside = 2;
    myFloat = .2;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[patchconstantfunc("HullConst")]
[outputcontrolpoints(3)]
ControlPoint Hull (InputPatch<ControlPoint,3> v, uint id : SV_OutputControlPointID) { return v[id]; }
