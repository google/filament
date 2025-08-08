               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %vert "main" %gl_Position
          %4 = OpString "Foo %f %f"
               OpSource HLSL 600
               OpName %vert "vert"
               OpDecorate %gl_Position BuiltIn Position
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
          %9 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
         %13 = OpTypeFunction %v4float
%gl_Position = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %vert = OpFunction %void None %12
         %15 = OpLabel
         %16 = OpVariable %_ptr_Function_v4float Function
         %17 = OpExtInst %void %1 1 %4 %float_1 %float_2
               OpStore %16 %9
               OpStore %gl_Position %9
               OpReturn
               OpFunctionEnd
