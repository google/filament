; SPIR-V
; Version: 1.0
; Generator: Wine VKD3D Shader Compiler; 1
; Bound: 13
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main" %8 %9 %11 %12
               OpName %1 "main"
               OpName %8 "v0"
               OpName %9 "v1"
               OpName %11 "o0"
               OpName %12 "o1"
               OpDecorate %8 Location 0
               OpDecorate %9 Location 1
               OpDecorate %11 BuiltIn Position
               OpDecorate %12 Location 1
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %5 = OpTypeFloat 32
          %6 = OpTypeVector %5 4
          %7 = OpTypePointer Input %6
          %8 = OpVariable %7 Input
          %9 = OpVariable %7 Input
         %10 = OpTypePointer Output %6
         %11 = OpVariable %10 Output
         %12 = OpVariable %10 Output
          %1 = OpFunction %2 None %3
          %4 = OpLabel
               OpCopyMemory %11 %8
               OpCopyMemory %12 %9
               OpReturn
               OpFunctionEnd
