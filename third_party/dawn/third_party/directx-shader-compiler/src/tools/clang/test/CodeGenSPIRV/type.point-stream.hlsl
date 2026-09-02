// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint Geometry %main "main"
// CHECK-SAME: %gl_Position %out_var_COLOR0 %out_var_TEXCOORD0

// CHECK: OpExecutionMode %main OutputPoints

// CHECK: OpDecorate %gl_Position BuiltIn Position
// CHECK: OpDecorate %out_var_COLOR0 Location 0
// CHECK: OpDecorate %out_var_TEXCOORD0 Location 1

// CHECK:               %GS_OUT = OpTypeStruct %v4float %v4float %v2float
// CHECK: %_ptr_Function_GS_OUT = OpTypePointer Function %GS_OUT

// CHECK:       %gl_Position = OpVariable %_ptr_Output_v4float Output
// CHECK:    %out_var_COLOR0 = OpVariable %_ptr_Output_v4float Output
// CHECK: %out_var_TEXCOORD0 = OpVariable %_ptr_Output_v2float Output

// CHECK:  %src_main = OpFunction %void None {{%[0-9]+}}
// CHECK:        %id = OpFunctionParameter %_ptr_Function__arr_v4float_uint_3
// CHECK: %outstream = OpFunctionParameter %_ptr_Function_GS_OUT

struct GS_OUT
{
  float4 position : SV_POSITION;
  float4 color    : COLOR0;
  float2 uv       : TEXCOORD0;
};

[maxvertexcount(3)]
void main(triangle float4 id[3] : VertexID, inout PointStream <GS_OUT> outstream)
{
}
