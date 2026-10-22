// RUN: %dxc -T hs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

#include "bezier_common_hull.hlsli"

// Test: PCF takes the output (OutputPatch) of the main entry point function.


// CHECK:             %_arr_BEZIER_CONTROL_POINT_uint_3 = OpTypeArray %BEZIER_CONTROL_POINT %uint_3
// CHECK: %_ptr_Function__arr_BEZIER_CONTROL_POINT_uint_3 = OpTypePointer Function %_arr_BEZIER_CONTROL_POINT_uint_3
// CHECK:                                 [[fType:%[0-9]+]] = OpTypeFunction %HS_CONSTANT_DATA_OUTPUT

// CHECK:                    %main = OpFunction %void None {{%[0-9]+}}
// CHECK: %temp_var_hullMainRetVal = OpVariable %_ptr_Function__arr_BEZIER_CONTROL_POINT_uint_3 Function

// CHECK:      [[mainResult:%[0-9]+]] = OpFunctionCall %BEZIER_CONTROL_POINT %src_main %param_var_ip %param_var_i %param_var_PatchID

// CHECK:      [[output_patch_0:%[0-9]+]] = OpAccessChain %_ptr_Function_BEZIER_CONTROL_POINT %temp_var_hullMainRetVal %uint_0
// CHECK:    [[output_patch_0_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[output_patch_0]] %uint_0
// CHECK: [[out_var_BEZIERPOS_0:%[0-9]+]] = OpAccessChain %_ptr_Output_v3float %out_var_BEZIERPOS %uint_0
// CHECK:         [[BEZIERPOS_0:%[0-9]+]] = OpLoad %v3float [[out_var_BEZIERPOS_0]]
// CHECK:                                OpStore [[output_patch_0_0]] [[BEZIERPOS_0]]

// CHECK:      [[output_patch_1:%[0-9]+]] = OpAccessChain %_ptr_Function_BEZIER_CONTROL_POINT %temp_var_hullMainRetVal %uint_1
// CHECK:    [[output_patch_1_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[output_patch_1]] %uint_0
// CHECK: [[out_var_BEZIERPOS_1:%[0-9]+]] = OpAccessChain %_ptr_Output_v3float %out_var_BEZIERPOS %uint_1
// CHECK:         [[BEZIERPOS_1:%[0-9]+]] = OpLoad %v3float [[out_var_BEZIERPOS_1]]
// CHECK:                                OpStore [[output_patch_1_0]] [[BEZIERPOS_1]]

// CHECK:      [[output_patch_2:%[0-9]+]] = OpAccessChain %_ptr_Function_BEZIER_CONTROL_POINT %temp_var_hullMainRetVal %uint_2
// CHECK:    [[output_patch_2_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[output_patch_2]] %uint_0
// CHECK: [[out_var_BEZIERPOS_2:%[0-9]+]] = OpAccessChain %_ptr_Output_v3float %out_var_BEZIERPOS %uint_2
// CHECK:         [[BEZIERPOS_2:%[0-9]+]] = OpLoad %v3float [[out_var_BEZIERPOS_2]]
// CHECK:                                OpStore [[output_patch_2_0]] [[BEZIERPOS_2]]

// CHECK:                 {{%[0-9]+}} = OpFunctionCall %HS_CONSTANT_DATA_OUTPUT %PCF %temp_var_hullMainRetVal

// CHECK:      %PCF = OpFunction %HS_CONSTANT_DATA_OUTPUT None [[fType]]

HS_CONSTANT_DATA_OUTPUT PCF(OutputPatch<BEZIER_CONTROL_POINT, 3> op) {
  HS_CONSTANT_DATA_OUTPUT Output;
  // Must initialize Edges and Inside; otherwise HLSL validation will fail.
  Output.Edges[0]  = 1.0;
  Output.Edges[1]  = 2.0;
  Output.Edges[2]  = 3.0;
  Output.Edges[3]  = 4.0;
  Output.Inside[0] = 5.0;
  Output.Inside[1] = 6.0;
  return Output;
}

[domain("isoline")]
[partitioning("fractional_odd")]
[outputtopology("line")]
[outputcontrolpoints(3)]
[patchconstantfunc("PCF")]
BEZIER_CONTROL_POINT main(InputPatch<VS_CONTROL_POINT_OUTPUT, 3> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID) {
  VS_CONTROL_POINT_OUTPUT vsOutput;
  BEZIER_CONTROL_POINT result;
  result.vPosition = vsOutput.vPosition;
  return result;
}
