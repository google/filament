; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 23
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %in_var_POSITION %gl_Position
          %4 = OpString ".\\push-constant-row-major-matrix.hlsl"
               OpSource HLSL 600 %4 "
struct Matrix {
    float4x4 transform;
};

[[vk::push_constant]] Matrix matrix_constants;

float4 main(float4 in_position : POSITION) : SV_Position {
    return mul(matrix_constants.transform, in_position);
}
"
               OpName %type_PushConstant_Matrix "type.PushConstant.Matrix"
               OpMemberName %type_PushConstant_Matrix 0 "transform"
               OpName %matrix_constants "matrix_constants"
               OpName %in_var_POSITION "in.var.POSITION"
               OpName %main "main"
               OpDecorateString %in_var_POSITION UserSemantic "POSITION"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorateString %gl_Position UserSemantic "SV_Position"
               OpDecorate %in_var_POSITION Location 0
               OpMemberDecorate %type_PushConstant_Matrix 0 Offset 0
               OpMemberDecorate %type_PushConstant_Matrix 0 MatrixStride 16
               OpMemberDecorate %type_PushConstant_Matrix 0 RowMajor
               OpDecorate %type_PushConstant_Matrix Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
%type_PushConstant_Matrix = OpTypeStruct %mat4v4float
%_ptr_PushConstant_type_PushConstant_Matrix = OpTypePointer PushConstant %type_PushConstant_Matrix
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_PushConstant_mat4v4float = OpTypePointer PushConstant %mat4v4float
%matrix_constants = OpVariable %_ptr_PushConstant_type_PushConstant_Matrix PushConstant
%in_var_POSITION = OpVariable %_ptr_Input_v4float Input
%gl_Position = OpVariable %_ptr_Output_v4float Output
               OpLine %4 8 1
       %main = OpFunction %void None %16
               OpNoLine
         %18 = OpLabel
               OpLine %4 8 1
         %19 = OpLoad %v4float %in_var_POSITION
               OpLine %4 9 16
         %20 = OpAccessChain %_ptr_PushConstant_mat4v4float %matrix_constants %int_0
               OpLine %4 9 33
         %21 = OpLoad %mat4v4float %20
               OpLine %4 9 12
         %22 = OpVectorTimesMatrix %v4float %19 %21
               OpLine %4 8 1
               OpStore %gl_Position %22
               OpLine %4 10 1
               OpReturn
               OpFunctionEnd