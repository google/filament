; SPIR-V
; Version: 1.3
; Generator: Google spiregg; 0
; Bound: 22
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
               OpExtension "SPV_GOOGLE_user_type"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %out_var_SV_Target
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 660
               OpName %type_StructuredBuffer_Data "type.StructuredBuffer.Data"
               OpName %Data "Data"
               OpMemberName %Data 0 "Color"
               OpName %Colors "Colors"
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpName %main "main"
               OpDecorateString %out_var_SV_Target UserSemantic "SV_Target"
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %Colors DescriptorSet 0
               OpDecorate %Colors Binding 0
               OpMemberDecorate %Data 0 Offset 0
               OpDecorate %_runtimearr_Data ArrayStride 16
               OpMemberDecorate %type_StructuredBuffer_Data 0 Offset 0
               OpMemberDecorate %type_StructuredBuffer_Data 0 NonWritable
               OpDecorate %type_StructuredBuffer_Data BufferBlock
               OpDecorateString %Colors UserTypeGOOGLE "structuredbuffer:<Data>"
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %Data = OpTypeStruct %v4float
%_runtimearr_Data = OpTypeRuntimeArray %Data
%type_StructuredBuffer_Data = OpTypeStruct %_runtimearr_Data
%_ptr_Uniform_type_StructuredBuffer_Data = OpTypePointer Uniform %type_StructuredBuffer_Data
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
     %Colors = OpVariable %_ptr_Uniform_type_StructuredBuffer_Data Uniform
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %17
         %19 = OpLabel
         %20 = OpAccessChain %_ptr_Uniform_v4float %Colors %int_0 %uint_3 %int_0
         %21 = OpLoad %v4float %20
               OpStore %out_var_SV_Target %21
               OpReturn
               OpFunctionEnd
