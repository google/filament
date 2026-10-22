; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 2
; Bound: 29
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %_main_ "@main("
               OpName %Foobar "Foobar"
               OpMemberName %Foobar 0 "@data"
               OpName %Foobar_0 "Foobar"
               OpName %Foobaz "Foobaz"
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %_runtimearr_v4float ArrayStride 16
               OpMemberDecorate %Foobar 0 Offset 0
               OpDecorate %Foobar BufferBlock
               OpDecorate %Foobar_0 DescriptorSet 0
               OpDecorate %Foobar_0 Binding 0
               OpDecorate %Foobaz DescriptorSet 0
               OpDecorate %Foobaz Binding 1
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %8 = OpTypeFunction %v4float
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
     %Foobar = OpTypeStruct %_runtimearr_v4float
%_ptr_Uniform_Foobar = OpTypePointer Uniform %Foobar
   %Foobar_0 = OpVariable %_ptr_Uniform_Foobar Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
     %Foobaz = OpVariable %_ptr_Uniform_Foobar Uniform
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %28 = OpFunctionCall %v4float %_main_
               OpStore %_entryPointOutput %28
               OpReturn
               OpFunctionEnd
     %_main_ = OpFunction %v4float None %8
         %10 = OpLabel
         %18 = OpAccessChain %_ptr_Uniform_v4float %Foobar_0 %int_0 %int_0
         %19 = OpLoad %v4float %18
         %21 = OpAccessChain %_ptr_Uniform_v4float %Foobaz %int_0 %int_0
         %22 = OpLoad %v4float %21
         %23 = OpFAdd %v4float %19 %22
               OpReturnValue %23
               OpFunctionEnd
