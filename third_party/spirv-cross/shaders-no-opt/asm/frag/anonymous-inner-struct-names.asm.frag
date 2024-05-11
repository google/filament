; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 27
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %_
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpMemberName %AA 0 "foo"
               OpMemberName %AB 0 "foo"
               OpMemberName %A 0 "_aa"
               OpMemberName %A 1 "ab"
               OpMemberName %BA 0 "foo"
               OpMemberName %BB 0 "foo"
               OpMemberName %B 0 "_ba"
               OpMemberName %B 1 "bb"
               OpName %VertexData "VertexData"
               OpMemberName %VertexData 0 "_a"
               OpMemberName %VertexData 1 "b"
               OpName %_ ""
               OpMemberName %CA 0 "foo"
               OpMemberName %C 0 "_ca"
               OpMemberName %DA 0 "foo"
               OpMemberName %D 0 "da"
               OpName %UBO "UBO"
               OpMemberName %UBO 0 "_c"
               OpMemberName %UBO 1 "d"
               OpName %__0 ""
               OpMemberName %E 0 "a"
               OpName %SSBO "SSBO"
               ;OpMemberName %SSBO 0 "e" Test that we don't try to assign bogus aliases.
               OpMemberName %SSBO 1 "_e"
               OpMemberName %SSBO 2 "f"
               OpName %__1 ""
               OpDecorate %VertexData Block
               OpDecorate %_ Location 0
               OpMemberDecorate %CA 0 Offset 0
               OpMemberDecorate %C 0 Offset 0
               OpMemberDecorate %DA 0 Offset 0
               OpMemberDecorate %D 0 Offset 0
               OpMemberDecorate %UBO 0 Offset 0
               OpMemberDecorate %UBO 1 Offset 16
               OpDecorate %UBO Block
               OpDecorate %__0 DescriptorSet 0
               OpDecorate %__0 Binding 0
               OpMemberDecorate %E 0 Offset 0
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpMemberDecorate %SSBO 2 Offset 8
               OpDecorate %SSBO BufferBlock
               OpDecorate %__1 DescriptorSet 0
               OpDecorate %__1 Binding 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
         %AA = OpTypeStruct %int
         %AB = OpTypeStruct %int
          %A = OpTypeStruct %AA %AB
         %BA = OpTypeStruct %int
         %BB = OpTypeStruct %int
          %B = OpTypeStruct %BA %BB
 %VertexData = OpTypeStruct %A %B
%_ptr_Input_VertexData = OpTypePointer Input %VertexData
          %_ = OpVariable %_ptr_Input_VertexData Input
         %CA = OpTypeStruct %int
          %C = OpTypeStruct %CA
         %DA = OpTypeStruct %int
          %D = OpTypeStruct %DA
        %UBO = OpTypeStruct %C %D
%_ptr_Uniform_UBO = OpTypePointer Uniform %UBO
        %__0 = OpVariable %_ptr_Uniform_UBO Uniform
          %E = OpTypeStruct %int
       %SSBO = OpTypeStruct %E %E %E
%_ptr_Uniform_SSBO = OpTypePointer Uniform %SSBO
        %__1 = OpVariable %_ptr_Uniform_SSBO Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
