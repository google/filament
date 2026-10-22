; SPIR-V
; Version: 1.0
; Generator: Google Shaderc over Glslang; 10
; Bound: 19
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 330
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %sw "sw"
               OpName %result "result"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
     %int_42 = OpConstant %int 42
      %int_0 = OpConstant %int 0
    %int_420 = OpConstant %int 420
       %main = OpFunction %void None %3
          %5 = OpLabel
         %sw = OpVariable %_ptr_Function_int Function
     %result = OpVariable %_ptr_Function_int Function
               OpStore %sw %int_42
               OpStore %result %int_0
         %12 = OpLoad %int %sw
               OpSelectionMerge %16 None
               OpSwitch %12 %16 -42 %13 420 %14 -1234 %15
         %13 = OpLabel
               OpStore %result %int_42
               OpBranch %14
         %14 = OpLabel
               OpStore %result %int_420
               OpBranch %15
         %15 = OpLabel
               OpStore %result %int_420
               OpBranch %16
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
