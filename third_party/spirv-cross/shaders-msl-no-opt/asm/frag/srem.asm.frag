; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 30
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %oA %A %B %oB %C %D
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %oA "oA"
               OpName %A "A"
               OpName %B "B"
               OpName %oB "oB"
               OpName %C "C"
               OpName %D "D"
               OpDecorate %oA Location 0
               OpDecorate %A Flat
               OpDecorate %A Location 0
               OpDecorate %B Flat
               OpDecorate %B Location 1
               OpDecorate %oB Location 1
               OpDecorate %C Flat
               OpDecorate %C Location 2
               OpDecorate %D Flat
               OpDecorate %D Location 3
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Output_int = OpTypePointer Output %int
         %oA = OpVariable %_ptr_Output_int Output
%_ptr_Input_int = OpTypePointer Input %int
          %A = OpVariable %_ptr_Input_int Input
       %uint = OpTypeInt 32 0
%_ptr_Input_uint = OpTypePointer Input %uint
          %B = OpVariable %_ptr_Input_uint Input
%_ptr_Output_uint = OpTypePointer Output %uint
         %oB = OpVariable %_ptr_Output_uint Output
          %C = OpVariable %_ptr_Input_int Input
          %D = OpVariable %_ptr_Input_uint Input
       %main = OpFunction %void None %10
         %17 = OpLabel
         %18 = OpLoad %int %A
         %19 = OpLoad %uint %B
         %20 = OpLoad %int %C
         %21 = OpLoad %uint %D
         %22 = OpSRem %uint %18 %19
               OpStore %oB %22
         %23 = OpSRem %uint %18 %20
               OpStore %oB %23
         %24 = OpSRem %uint %19 %21
               OpStore %oB %24
         %25 = OpSRem %uint %19 %18
               OpStore %oB %25
         %26 = OpSRem %int %18 %19
               OpStore %oA %26
         %27 = OpSRem %int %18 %20
               OpStore %oA %27
         %28 = OpSRem %int %19 %21
               OpStore %oA %28
         %29 = OpSRem %int %19 %18
               OpStore %oA %29
               OpReturn
               OpFunctionEnd
