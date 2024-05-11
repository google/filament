; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 21
; Schema: 0
               OpCapability Shader
               OpCapability Int64
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
          %6 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %long = OpTypeInt 64 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_long = OpTypePointer Function %long
     %int_42 = OpConstant %int 42
      %int_0 = OpConstant %int 0
    %int_420 = OpConstant %int 420
    %long_42 = OpConstant %long 42
       %main = OpFunction %void None %6
         %15 = OpLabel
         %sw = OpVariable %_ptr_Function_long Function
     %result = OpVariable %_ptr_Function_int Function
               OpStore %sw %long_42
               OpStore %result %int_0
         %16 = OpLoad %long %sw
               OpSelectionMerge %17 None
               OpSwitch %16 %17 -42 %18 420 %19 -34359738368 %20
         %18 = OpLabel
               OpStore %result %int_42
               OpBranch %19
         %19 = OpLabel
               OpStore %result %int_420
               OpBranch %20
         %20 = OpLabel
               OpStore %result %int_420
               OpBranch %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
