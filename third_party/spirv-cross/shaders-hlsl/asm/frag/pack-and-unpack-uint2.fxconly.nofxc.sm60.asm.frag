; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 34
; Schema: 0
               OpCapability Shader
               OpCapability Int64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_ARB_gpu_shader_int64"
               OpName %main "main"
               OpName %packed "packed"
               OpName %unpacked "unpacked"
               OpName %FragColor "FragColor"
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %ulong = OpTypeInt 64 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_18 = OpConstant %uint 18
    %uint_52 = OpConstant %uint 52
         %13 = OpConstantComposite %v2uint %uint_18 %uint_52
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
     %uint_0 = OpConstant %uint 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_1 = OpConstant %uint 1
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %3
          %5 = OpLabel
     %packed = OpVariable %_ptr_Function_ulong Function
   %unpacked = OpVariable %_ptr_Function_v2uint Function
         %14 = OpBitcast %ulong %13
               OpStore %packed %14
         %17 = OpLoad %ulong %packed
         %18 = OpBitcast %v2uint %17
               OpStore %unpacked %18
         %25 = OpAccessChain %_ptr_Function_uint %unpacked %uint_0
         %26 = OpLoad %uint %25
         %27 = OpConvertUToF %float %26
         %29 = OpAccessChain %_ptr_Function_uint %unpacked %uint_1
         %30 = OpLoad %uint %29
         %31 = OpConvertUToF %float %30
         %33 = OpCompositeConstruct %v4float %27 %31 %float_1 %float_1
               OpStore %FragColor %33
               OpReturn
               OpFunctionEnd
