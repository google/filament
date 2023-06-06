; SPIR-V
; Version: 1.1
; Generator: Google rspirv; 0
; Bound: 17
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %2 "main"
               OpName %Test "Test"
               OpName %t "t"
               OpName %retvar "retvar"
               OpName %main "main"
               OpName %retvar_0 "retvar"
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
       %Test = OpTypeStruct
%_ptr_Function_Test = OpTypePointer Function %Test
%_ptr_Function_void = OpTypePointer Function %void
          %2 = OpFunction %void None %6
          %7 = OpLabel
          %t = OpVariable %_ptr_Function_Test Function
     %retvar = OpVariable %_ptr_Function_void Function
               OpBranch %4
          %4 = OpLabel
         %13 = OpCompositeConstruct %Test
               OpStore %t %13
               OpReturn
               OpFunctionEnd
       %main = OpFunction %void None %6
         %15 = OpLabel
   %retvar_0 = OpVariable %_ptr_Function_void Function
               OpBranch %14
         %14 = OpLabel
               OpReturn
               OpFunctionEnd
