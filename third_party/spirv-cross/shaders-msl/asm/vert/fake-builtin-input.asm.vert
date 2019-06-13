; SPIR-V
; Version: 1.3
; Generator: Google spiregg; 0
; Bound: 29
; Schema: 0
               OpCapability Shader
               OpCapability Float16
               OpCapability StorageInputOutput16
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %vertexShader "main" %in_var_POSITION %gl_Position %gl_FragCoord %out_var_SV_Target
               OpEntryPoint Fragment %fragmentShader "fragmentShader" %in_var_POSITION %gl_Position %gl_FragCoord %out_var_SV_Target
               OpExecutionMode %fragmentShader OriginUpperLeft
               OpSource HLSL 640
               OpName %in_var_POSITION "in.var.POSITION"
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpName %vertexShader "vertexShader"
               OpName %fragmentShader "fragmentShader"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %in_var_POSITION Location 0
               OpDecorate %out_var_SV_Target Location 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
       %half = OpTypeFloat 16
%half_0x1p_0 = OpConstant %half 0x1p+0
%half_0x0p_0 = OpConstant %half 0x0p+0
     %v4half = OpTypeVector %half 4
         %14 = OpConstantComposite %v4half %half_0x1p_0 %half_0x0p_0 %half_0x1p_0 %half_0x1p_0
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4half = OpTypePointer Output %v4half
       %void = OpTypeVoid
         %22 = OpTypeFunction %void
%in_var_POSITION = OpVariable %_ptr_Input_v2float Input
%gl_Position = OpVariable %_ptr_Output_v4float Output
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4half Output
%vertexShader = OpFunction %void None %22
         %23 = OpLabel
         %24 = OpLoad %v2float %in_var_POSITION
         %25 = OpCompositeExtract %float %24 0
         %26 = OpCompositeExtract %float %24 1
         %27 = OpCompositeConstruct %v4float %25 %26 %float_0 %float_1
               OpStore %gl_Position %27
               OpReturn
               OpFunctionEnd
%fragmentShader = OpFunction %void None %22
         %28 = OpLabel
               OpStore %out_var_SV_Target %14
               OpReturn
               OpFunctionEnd
