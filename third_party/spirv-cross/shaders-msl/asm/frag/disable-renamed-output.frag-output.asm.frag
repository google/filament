; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 8
; Bound: 37
; Schema: 0
               OpCapability Shader
               OpCapability StencilExportEXT
               OpExtension "SPV_EXT_shader_stencil_export"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %o0 %o1 %o2 %o3 %o4 %o5 %o6 %o7 %oDepth %oStencil
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main DepthReplacing
               OpSource GLSL 450
               OpSourceExtension "GL_ARB_shader_stencil_export"
               OpName %main "main"
               OpName %o0 "o0"
               OpName %o1 "o1"
               OpName %o2 "o2"
               OpName %o3 "o3"
               OpName %o4 "o4"
               OpName %o5 "o5"
               OpName %o6 "o6"
               OpName %o7 "o7"
               OpName %oDepth "oDepth"
               OpName %oStencil "oStencil"
               OpDecorate %o0 Location 0
               OpDecorate %o1 Location 1
               OpDecorate %o2 Location 2
               OpDecorate %o3 Location 3
               OpDecorate %o4 Location 4
               OpDecorate %o5 Location 5
               OpDecorate %o6 Location 6
               OpDecorate %o7 Location 7
               OpDecorate %oDepth BuiltIn FragDepth
               OpDecorate %oStencil BuiltIn FragStencilRefEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
         %o0 = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
         %12 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
         %o1 = OpVariable %_ptr_Output_v4float Output
         %14 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
         %o2 = OpVariable %_ptr_Output_v4float Output
         %16 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
         %o3 = OpVariable %_ptr_Output_v4float Output
         %18 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
         %o4 = OpVariable %_ptr_Output_v4float Output
  %float_0_5 = OpConstant %float 0.5
         %21 = OpConstantComposite %v4float %float_1 %float_0 %float_1 %float_0_5
         %o5 = OpVariable %_ptr_Output_v4float Output
 %float_0_25 = OpConstant %float 0.25
         %24 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
         %o6 = OpVariable %_ptr_Output_v4float Output
 %float_0_75 = OpConstant %float 0.75
         %27 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
         %o7 = OpVariable %_ptr_Output_v4float Output
         %29 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_float = OpTypePointer Output %float
     %oDepth = OpVariable %_ptr_Output_float Output
%float_0_899999976 = OpConstant %float 0.899999976
        %int = OpTypeInt 32 1
%_ptr_Output_int = OpTypePointer Output %int
   %oStencil = OpVariable %_ptr_Output_int Output
    %int_127 = OpConstant %int 127
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %o0 %12
               OpStore %o1 %14
               OpStore %o2 %16
               OpStore %o3 %18
               OpStore %o4 %21
               OpStore %o5 %24
               OpStore %o6 %27
               OpStore %o7 %29
               OpStore %oDepth %float_0_899999976
               OpStore %oStencil %int_127
               OpReturn
               OpFunctionEnd
