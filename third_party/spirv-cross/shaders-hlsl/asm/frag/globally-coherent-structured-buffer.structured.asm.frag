; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 20
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
               OpExtension "SPV_GOOGLE_user_type"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %out_var_SV_Target0
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_RWStructuredBuffer_v4float "type.RWStructuredBuffer.v4float"
               OpName %TestBuffer "TestBuffer"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %main "main"
               OpDecorateString %out_var_SV_Target0 UserSemantic "SV_Target0"
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %TestBuffer DescriptorSet 0
               OpDecorate %TestBuffer Binding 0
               OpDecorate %TestBuffer Coherent
               OpDecorate %_runtimearr_v4float ArrayStride 16
               OpMemberDecorate %type_RWStructuredBuffer_v4float 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_v4float BufferBlock
               OpDecorateString %TestBuffer UserTypeGOOGLE "globallycoherent rwstructuredbuffer:<float4>"
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
%type_RWStructuredBuffer_v4float = OpTypeStruct %_runtimearr_v4float
%_ptr_Uniform_type_RWStructuredBuffer_v4float = OpTypePointer Uniform %type_RWStructuredBuffer_v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
 %TestBuffer = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_v4float Uniform
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %15
         %17 = OpLabel
         %18 = OpAccessChain %_ptr_Uniform_v4float %TestBuffer %int_0 %uint_0
         %19 = OpLoad %v4float %18
               OpStore %out_var_SV_Target0 %19
               OpReturn
               OpFunctionEnd