; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 36
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %vid_1 %iid_1 %_entryPointOutput
               OpSource HLSL 500
               OpName %main "main"
               OpName %_main_u1_u1_ "@main(u1;u1;"
               OpName %vid "vid"
               OpName %iid "iid"
               OpName %vid_0 "vid"
               OpName %vid_1 "vid"
               OpName %iid_0 "iid"
               OpName %iid_1 "iid"
               OpName %_entryPointOutput "@entryPointOutput"
               OpName %param "param"
               OpName %param_0 "param"
               OpDecorate %vid_1 BuiltIn VertexIndex
               OpDecorate %iid_1 BuiltIn InstanceIndex
               OpDecorate %_entryPointOutput BuiltIn Position
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
         %10 = OpTypeFunction %v4float %_ptr_Function_uint %_ptr_Function_uint
%_ptr_Input_uint = OpTypePointer Input %uint
      %vid_1 = OpVariable %_ptr_Input_uint Input
      %iid_1 = OpVariable %_ptr_Input_uint Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
      %vid_0 = OpVariable %_ptr_Function_uint Function
      %iid_0 = OpVariable %_ptr_Function_uint Function
      %param = OpVariable %_ptr_Function_uint Function
    %param_0 = OpVariable %_ptr_Function_uint Function
         %25 = OpLoad %uint %vid_1
               OpStore %vid_0 %25
         %28 = OpLoad %uint %iid_1
               OpStore %iid_0 %28
         %32 = OpLoad %uint %vid_0
               OpStore %param %32
         %34 = OpLoad %uint %iid_0
               OpStore %param_0 %34
         %35 = OpFunctionCall %v4float %_main_u1_u1_ %param %param_0
               OpStore %_entryPointOutput %35
               OpReturn
               OpFunctionEnd
%_main_u1_u1_ = OpFunction %v4float None %10
        %vid = OpFunctionParameter %_ptr_Function_uint
        %iid = OpFunctionParameter %_ptr_Function_uint
         %14 = OpLabel
         %15 = OpLoad %uint %vid
         %16 = OpLoad %uint %iid
         %17 = OpIAdd %uint %15 %16
         %18 = OpConvertUToF %float %17
         %19 = OpCompositeConstruct %v4float %18 %18 %18 %18
               OpReturnValue %19
               OpFunctionEnd
