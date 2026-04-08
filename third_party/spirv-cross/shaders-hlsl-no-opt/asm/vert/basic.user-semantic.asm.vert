; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 15
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %in_var_B %gl_Position %out_var_A
               OpSource HLSL 600
               OpName %in_var_B "in.var.B"
               OpName %out_var_A "out.var.A"
               OpName %main "main"
               OpDecorateString %in_var_B UserSemantic "B"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorateString %gl_Position UserSemantic "SV_Position"
               OpDecorateString %out_var_A UserSemantic "A"
               OpDecorate %in_var_B Location 0
               OpDecorate %out_var_A Location 0
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %v4float = OpTypeVector %float 4
          %8 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
   %in_var_B = OpVariable %_ptr_Input_v4float Input
%gl_Position = OpVariable %_ptr_Output_v4float Output
  %out_var_A = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %12
         %13 = OpLabel
         %14 = OpLoad %v4float %in_var_B
               OpStore %gl_Position %8
               OpStore %out_var_A %14
               OpReturn
               OpFunctionEnd
