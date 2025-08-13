; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 19
; Schema: 0
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %LOD %Coord
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %LOD "LOD"
               OpName %Coord "Coord"
               OpName %Texture0 "Texture0"
               OpName %Texture1 "Texture1"
               OpName %Texture2 "Texture2"
               OpName %Texture3 "Texture3"
               OpName %Texture4 "Texture4"
               OpDecorate %LOD Location 0
               OpDecorate %Coord Location 0
               OpDecorate %Texture0 DescriptorSet 0
               OpDecorate %Texture0 Binding 0
               OpDecorate %Texture1 DescriptorSet 0
               OpDecorate %Texture1 Binding 1
               OpDecorate %Texture2 DescriptorSet 0
               OpDecorate %Texture2 Binding 2
               OpDecorate %Texture3 DescriptorSet 0
               OpDecorate %Texture3 Binding 3
               OpDecorate %Texture4 DescriptorSet 0
               OpDecorate %Texture4 Binding 4
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1  
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v4float = OpTypeVector %float 4
%_ptr_Output_v2float = OpTypePointer Output %v2float
%_ptr_Input_v4float = OpTypePointer Input %v4float
       %LOD = OpVariable %_ptr_Output_v2float Output
      %Coord = OpVariable %_ptr_Input_v4float Input
      %tex2d = OpTypeImage %float 2D   0 0 0 1 Unknown
   %tex2darr = OpTypeImage %float 2D   0 1 0 1 Unknown
      %tex3d = OpTypeImage %float 3D   0 0 0 1 Unknown
    %texcube = OpTypeImage %float Cube 0 0 0 1 Unknown
 %texcubearr = OpTypeImage %float Cube 0 1 0 1 Unknown
     %samp2d = OpTypeSampledImage %tex2d
  %samp2darr = OpTypeSampledImage %tex2darr
     %samp3d = OpTypeSampledImage %tex3d
   %sampcube = OpTypeSampledImage %texcube
%sampcubearr = OpTypeSampledImage %texcubearr
     %pucs2d = OpTypePointer UniformConstant %samp2d
  %pucs2darr = OpTypePointer UniformConstant %samp2darr
     %pucs3d = OpTypePointer UniformConstant %samp3d
   %pucscube = OpTypePointer UniformConstant %sampcube
%pucscubearr = OpTypePointer UniformConstant %sampcubearr
   %Texture0 = OpVariable %pucs2d      UniformConstant
   %Texture1 = OpVariable %pucs2darr   UniformConstant
   %Texture2 = OpVariable %pucs3d      UniformConstant
   %Texture3 = OpVariable %pucscube    UniformConstant
   %Texture4 = OpVariable %pucscubearr UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
      %coord = OpLoad %v4float %Coord
         %t0 = OpLoad %samp2d      %Texture0
         %t1 = OpLoad %samp2darr   %Texture1
         %t2 = OpLoad %samp3d      %Texture2
         %t3 = OpLoad %sampcube    %Texture3
         %t4 = OpLoad %sampcubearr %Texture4
         %l0 = OpImageQueryLod %v2float %t0 %coord
         %l1 = OpImageQueryLod %v2float %t1 %coord
         %l2 = OpImageQueryLod %v2float %t2 %coord
         %l3 = OpImageQueryLod %v2float %t3 %coord
         %l4 = OpImageQueryLod %v2float %t4 %coord
         %10 = OpFAdd %v2float %l0 %l1
         %11 = OpFAdd %v2float %l2 %l3
         %12 = OpFAdd %v2float %10 %11
         %13 = OpFAdd %v2float %12 %l4
               OpStore %LOD %13
               OpReturn
               OpFunctionEnd
