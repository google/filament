; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 36
; Schema: 0
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %b "b"
               OpName %uSampler2D "uSampler2D"
               OpName %c "c"
               OpName %uSampler2DMS "uSampler2DMS"
               OpName %l1 "l1"
               OpName %s0 "s0"
               OpDecorate %uSampler2D DescriptorSet 0
               OpDecorate %uSampler2D Binding 0
               OpDecorate %uSampler2DMS DescriptorSet 0
               OpDecorate %uSampler2DMS Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
      %float = OpTypeFloat 32
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %11
 %uSampler2D = OpVariable %_ptr_UniformConstant_12 UniformConstant
      %int_0 = OpConstant %int 0
         %20 = OpTypeImage %float 2D 0 0 1 1 Unknown
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %20
%uSampler2DMS = OpVariable %_ptr_UniformConstant_21 UniformConstant
%_ptr_Function_int = OpTypePointer Function %int
       %main = OpFunction %void None %3
          %5 = OpLabel
          %b = OpVariable %_ptr_Function_v2int Function
          %c = OpVariable %_ptr_Function_v2int Function
         %l1 = OpVariable %_ptr_Function_int Function
         %s0 = OpVariable %_ptr_Function_int Function
         %15 = OpLoad %11 %uSampler2D
         %18 = OpImageQuerySizeLod %v2int %15 %int_0
               OpStore %b %18
         %24 = OpLoad %20 %uSampler2DMS
         %26 = OpImageQuerySize %v2int %24
               OpStore %c %26
         %29 = OpLoad %11 %uSampler2D
         %31 = OpImageQueryLevels %int %29
               OpStore %l1 %31
         %33 = OpLoad %20 %uSampler2DMS
         %35 = OpImageQuerySamples %int %33
               OpStore %s0 %35
               OpReturn
               OpFunctionEnd
