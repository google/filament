; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 247
; Schema: 0
               OpCapability Shader
               OpCapability Sampled1D
               OpCapability SampledCubeArray
               OpCapability SampledBuffer
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %c "c"
               OpName %tex1d "tex1d"
               OpName %tex2d "tex2d"
               OpName %tex3d "tex3d"
               OpName %texCube "texCube"
               OpName %tex2dArray "tex2dArray"
               OpName %texCubeArray "texCubeArray"
               OpName %depth2d "depth2d"
               OpName %depthCube "depthCube"
               OpName %depth2dArray "depth2dArray"
               OpName %depthCubeArray "depthCubeArray"
               OpName %texBuffer "texBuffer"
               OpName %tex1dSamp "tex1dSamp"
               OpName %tex2dSamp "tex2dSamp"
               OpName %tex3dSamp "tex3dSamp"
               OpName %texCubeSamp "texCubeSamp"
               OpName %tex2dArraySamp "tex2dArraySamp"
               OpName %texCubeArraySamp "texCubeArraySamp"
               OpName %depth2dSamp "depth2dSamp"
               OpName %depthCubeSamp "depthCubeSamp"
               OpName %depth2dArraySamp "depth2dArraySamp"
               OpName %depthCubeArraySamp "depthCubeArraySamp"
               OpDecorate %tex1d DescriptorSet 0
               OpDecorate %tex1d Binding 0
               OpDecorate %tex2d DescriptorSet 0
               OpDecorate %tex2d Binding 1
               OpDecorate %tex3d DescriptorSet 0
               OpDecorate %tex3d Binding 2
               OpDecorate %texCube DescriptorSet 0
               OpDecorate %texCube Binding 3
               OpDecorate %tex2dArray DescriptorSet 0
               OpDecorate %tex2dArray Binding 4
               OpDecorate %texCubeArray DescriptorSet 0
               OpDecorate %texCubeArray Binding 5
               OpDecorate %depth2d DescriptorSet 0
               OpDecorate %depth2d Binding 7
               OpDecorate %depthCube DescriptorSet 0
               OpDecorate %depthCube Binding 8
               OpDecorate %depth2dArray DescriptorSet 0
               OpDecorate %depth2dArray Binding 9
               OpDecorate %depthCubeArray DescriptorSet 0
               OpDecorate %depthCubeArray Binding 10
               OpDecorate %texBuffer DescriptorSet 0
               OpDecorate %texBuffer Binding 6
               OpDecorate %tex1dSamp DescriptorSet 1
               OpDecorate %tex1dSamp Binding 0
               OpDecorate %tex2dSamp DescriptorSet 1
               OpDecorate %tex2dSamp Binding 1
               OpDecorate %tex3dSamp DescriptorSet 1
               OpDecorate %tex3dSamp Binding 2
               OpDecorate %texCubeSamp DescriptorSet 1
               OpDecorate %texCubeSamp Binding 3
               OpDecorate %tex2dArraySamp DescriptorSet 1
               OpDecorate %tex2dArraySamp Binding 4
               OpDecorate %texCubeArraySamp DescriptorSet 1
               OpDecorate %texCubeArraySamp Binding 5
               OpDecorate %depth2dSamp DescriptorSet 1
               OpDecorate %depth2dSamp Binding 7
               OpDecorate %depthCubeSamp DescriptorSet 1
               OpDecorate %depthCubeSamp Binding 8
               OpDecorate %depth2dArraySamp DescriptorSet 1
               OpDecorate %depth2dArraySamp Binding 9
               OpDecorate %depthCubeArraySamp DescriptorSet 1
               OpDecorate %depthCubeArraySamp Binding 10
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 1D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
         %12 = OpTypeSampler
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
      %tex1d = OpVariable %_ptr_UniformConstant_10 UniformConstant
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
  %tex1dSamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
    %float_0 = OpConstant %float 0
         %17 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %18 = OpTypeSampledImage %17
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
      %tex2d = OpVariable %_ptr_UniformConstant_17 UniformConstant
  %tex2dSamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
         %25 = OpTypeImage %float 3D 0 0 0 1 Unknown
         %26 = OpTypeSampledImage %25
%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %25
      %tex3d = OpVariable %_ptr_UniformConstant_25 UniformConstant
  %tex3dSamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
    %v3float = OpTypeVector %float 3
         %31 = OpConstantComposite %v3float %float_0 %float_0 %float_0
         %33 = OpTypeImage %float Cube 0 0 0 1 Unknown
         %34 = OpTypeSampledImage %33
%_ptr_UniformConstant_33 = OpTypePointer UniformConstant %33
    %texCube = OpVariable %_ptr_UniformConstant_33 UniformConstant
%texCubeSamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
         %39 = OpTypeImage %float 2D 0 1 0 1 Unknown
         %40 = OpTypeSampledImage %39
%_ptr_UniformConstant_39 = OpTypePointer UniformConstant %39
 %tex2dArray = OpVariable %_ptr_UniformConstant_39 UniformConstant
%tex2dArraySamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
         %45 = OpTypeImage %float Cube 0 1 0 1 Unknown
         %46 = OpTypeSampledImage %45
%_ptr_UniformConstant_45 = OpTypePointer UniformConstant %45
%texCubeArray = OpVariable %_ptr_UniformConstant_45 UniformConstant
%texCubeArraySamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
         %50 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %52 = OpTypeImage %float 2D 1 0 0 1 Unknown
         %53 = OpTypeSampledImage %52
%_ptr_UniformConstant_52 = OpTypePointer UniformConstant %52
    %depth2d = OpVariable %_ptr_UniformConstant_52 UniformConstant
%depth2dSamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
    %float_1 = OpConstant %float 1
         %58 = OpConstantComposite %v3float %float_0 %float_0 %float_1
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
         %65 = OpTypeImage %float Cube 1 0 0 1 Unknown
         %66 = OpTypeSampledImage %65
%_ptr_UniformConstant_65 = OpTypePointer UniformConstant %65
  %depthCube = OpVariable %_ptr_UniformConstant_65 UniformConstant
%depthCubeSamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
         %70 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
         %74 = OpTypeImage %float 2D 1 1 0 1 Unknown
         %75 = OpTypeSampledImage %74
%_ptr_UniformConstant_74 = OpTypePointer UniformConstant %74
%depth2dArray = OpVariable %_ptr_UniformConstant_74 UniformConstant
%depth2dArraySamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
         %82 = OpTypeImage %float Cube 1 1 0 1 Unknown
         %83 = OpTypeSampledImage %82
%_ptr_UniformConstant_82 = OpTypePointer UniformConstant %82
%depthCubeArray = OpVariable %_ptr_UniformConstant_82 UniformConstant
%depthCubeArraySamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
         %97 = OpConstantComposite %v2float %float_0 %float_1
         %98 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %v2int = OpTypeVector %int 2
        %138 = OpConstantComposite %v2int %int_0 %int_0
      %v3int = OpTypeVector %int 3
        %143 = OpConstantComposite %v3int %int_0 %int_0 %int_0
        %149 = OpTypeImage %float Buffer 0 0 0 1 Unknown
%_ptr_UniformConstant_149 = OpTypePointer UniformConstant %149
  %texBuffer = OpVariable %_ptr_UniformConstant_149 UniformConstant
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %main = OpFunction %void None %3
          %5 = OpLabel
          %c = OpVariable %_ptr_Function_v4float Function
         %13 = OpLoad %10 %tex1d
         %14 = OpLoad %12 %tex1dSamp
         %15 = OpSampledImage %11 %13 %14
         %16 = OpImageSampleImplicitLod %v4float %15 %float_0
               OpStore %c %16
         %19 = OpLoad %17 %tex2d
         %20 = OpLoad %12 %tex2dSamp
         %21 = OpSampledImage %18 %19 %20
         %24 = OpImageSampleImplicitLod %v4float %21 %23
               OpStore %c %24
         %27 = OpLoad %25 %tex3d
         %28 = OpLoad %12 %tex3dSamp
         %29 = OpSampledImage %26 %27 %28
         %32 = OpImageSampleImplicitLod %v4float %29 %31
               OpStore %c %32
         %35 = OpLoad %33 %texCube
         %36 = OpLoad %12 %texCubeSamp
         %37 = OpSampledImage %34 %35 %36
         %38 = OpImageSampleImplicitLod %v4float %37 %31
               OpStore %c %38
         %41 = OpLoad %39 %tex2dArray
         %42 = OpLoad %12 %tex2dArraySamp
         %43 = OpSampledImage %40 %41 %42
         %44 = OpImageSampleImplicitLod %v4float %43 %31
               OpStore %c %44
         %47 = OpLoad %45 %texCubeArray
         %48 = OpLoad %12 %texCubeArraySamp
         %49 = OpSampledImage %46 %47 %48
         %51 = OpImageSampleImplicitLod %v4float %49 %50
               OpStore %c %51
         %54 = OpLoad %52 %depth2d
         %55 = OpLoad %12 %depth2dSamp
         %56 = OpSampledImage %53 %54 %55
         %59 = OpCompositeExtract %float %58 2
         %60 = OpImageSampleDrefImplicitLod %float %56 %58 %59
         %64 = OpAccessChain %_ptr_Function_float %c %uint_0
               OpStore %64 %60
         %67 = OpLoad %65 %depthCube
         %68 = OpLoad %12 %depthCubeSamp
         %69 = OpSampledImage %66 %67 %68
         %71 = OpCompositeExtract %float %70 3
         %72 = OpImageSampleDrefImplicitLod %float %69 %70 %71
         %73 = OpAccessChain %_ptr_Function_float %c %uint_0
               OpStore %73 %72
         %76 = OpLoad %74 %depth2dArray
         %77 = OpLoad %12 %depth2dArraySamp
         %78 = OpSampledImage %75 %76 %77
         %79 = OpCompositeExtract %float %70 3
         %80 = OpImageSampleDrefImplicitLod %float %78 %70 %79
         %81 = OpAccessChain %_ptr_Function_float %c %uint_0
               OpStore %81 %80
         %84 = OpLoad %82 %depthCubeArray
         %85 = OpLoad %12 %depthCubeArraySamp
         %86 = OpSampledImage %83 %84 %85
         %87 = OpImageSampleDrefImplicitLod %float %86 %50 %float_1
         %88 = OpAccessChain %_ptr_Function_float %c %uint_0
               OpStore %88 %87
         %89 = OpLoad %10 %tex1d
         %90 = OpLoad %12 %tex1dSamp
         %91 = OpSampledImage %11 %89 %90
         %92 = OpImageSampleProjImplicitLod %v4float %91 %97
               OpStore %c %92
         %93 = OpLoad %17 %tex2d
         %94 = OpLoad %12 %tex2dSamp
         %95 = OpSampledImage %18 %93 %94
         %96 = OpImageSampleProjImplicitLod %v4float %95 %58
               OpStore %c %96
         %99 = OpLoad %25 %tex3d
        %100 = OpLoad %12 %tex3dSamp
        %101 = OpSampledImage %26 %99 %100
        %102 = OpImageSampleProjImplicitLod %v4float %101 %70
               OpStore %c %102
        %103 = OpLoad %52 %depth2d
        %104 = OpLoad %12 %depth2dSamp
        %105 = OpSampledImage %53 %103 %104
        %106 = OpCompositeExtract %float %98 2
        %107 = OpCompositeExtract %float %98 3
        %108 = OpCompositeInsert %v4float %107 %98 2
        %109 = OpImageSampleProjDrefImplicitLod %float %105 %108 %106
        %110 = OpAccessChain %_ptr_Function_float %c %uint_0
               OpStore %110 %109
        %111 = OpLoad %10 %tex1d
        %112 = OpLoad %12 %tex1dSamp
        %113 = OpSampledImage %11 %111 %112
        %114 = OpImageSampleExplicitLod %v4float %113 %float_0 Lod %float_0
               OpStore %c %114
        %115 = OpLoad %17 %tex2d
        %116 = OpLoad %12 %tex2dSamp
        %117 = OpSampledImage %18 %115 %116
        %118 = OpImageSampleExplicitLod %v4float %117 %23 Lod %float_0
               OpStore %c %118
        %119 = OpLoad %25 %tex3d
        %120 = OpLoad %12 %tex3dSamp
        %121 = OpSampledImage %26 %119 %120
        %122 = OpImageSampleExplicitLod %v4float %121 %31 Lod %float_0
               OpStore %c %122
        %123 = OpLoad %33 %texCube
        %124 = OpLoad %12 %texCubeSamp
        %125 = OpSampledImage %34 %123 %124
        %126 = OpImageSampleExplicitLod %v4float %125 %31 Lod %float_0
               OpStore %c %126
        %127 = OpLoad %39 %tex2dArray
        %128 = OpLoad %12 %tex2dArraySamp
        %129 = OpSampledImage %40 %127 %128
        %130 = OpImageSampleExplicitLod %v4float %129 %31 Lod %float_0
               OpStore %c %130
        %131 = OpLoad %45 %texCubeArray
        %132 = OpLoad %12 %texCubeArraySamp
        %133 = OpSampledImage %46 %131 %132
        %134 = OpImageSampleExplicitLod %v4float %133 %50 Lod %float_0
               OpStore %c %134
        %135 = OpLoad %52 %depth2d
        %136 = OpLoad %12 %depth2dSamp
        %137 = OpSampledImage %53 %135 %136
        %139 = OpCompositeExtract %float %58 2
        %140 = OpImageSampleDrefExplicitLod %float %137 %58 %139 Lod %float_0
        %141 = OpAccessChain %_ptr_Function_float %c %uint_0
               OpStore %141 %140
        %142 = OpLoad %10 %tex1d
        %144 = OpLoad %12 %tex1dSamp
        %145 = OpSampledImage %11 %142 %144
        %146 = OpImageSampleProjExplicitLod %v4float %145 %97 Lod %float_0
               OpStore %c %146
        %147 = OpLoad %17 %tex2d
        %148 = OpLoad %12 %tex2dSamp
        %150 = OpSampledImage %18 %147 %148
        %151 = OpImageSampleProjExplicitLod %v4float %150 %58 Lod %float_0
               OpStore %c %151
        %152 = OpLoad %25 %tex3d
        %153 = OpLoad %12 %tex3dSamp
        %154 = OpSampledImage %26 %152 %153
        %155 = OpImageSampleProjExplicitLod %v4float %154 %70 Lod %float_0
               OpStore %c %155
        %156 = OpLoad %52 %depth2d
        %157 = OpLoad %12 %depth2dSamp
        %158 = OpSampledImage %53 %156 %157
        %159 = OpCompositeExtract %float %98 2
        %160 = OpCompositeExtract %float %98 3
        %161 = OpCompositeInsert %v4float %160 %98 2
        %162 = OpImageSampleProjDrefExplicitLod %float %158 %161 %159 Lod %float_0
        %163 = OpAccessChain %_ptr_Function_float %c %uint_0
               OpStore %163 %162
        %164 = OpLoad %10 %tex1d
        %165 = OpImageFetch %v4float %164 %int_0 Lod %int_0
               OpStore %c %165
        %166 = OpLoad %17 %tex2d
        %167 = OpImageFetch %v4float %166 %138 Lod %int_0
               OpStore %c %167
        %168 = OpLoad %25 %tex3d
        %169 = OpImageFetch %v4float %168 %143 Lod %int_0
               OpStore %c %169
        %170 = OpLoad %39 %tex2dArray
        %171 = OpImageFetch %v4float %170 %143 Lod %int_0
               OpStore %c %171
        %172 = OpLoad %149 %texBuffer
        %173 = OpImageFetch %v4float %172 %int_0
               OpStore %c %173
        %174 = OpLoad %17 %tex2d
        %175 = OpLoad %12 %tex2dSamp
        %176 = OpSampledImage %18 %174 %175
        %177 = OpImageGather %v4float %176 %23 %int_0
               OpStore %c %177
        %178 = OpLoad %33 %texCube
        %179 = OpLoad %12 %texCubeSamp
        %180 = OpSampledImage %34 %178 %179
        %181 = OpImageGather %v4float %180 %31 %int_1
               OpStore %c %181
        %182 = OpLoad %39 %tex2dArray
        %183 = OpLoad %12 %tex2dArraySamp
        %184 = OpSampledImage %40 %182 %183
        %185 = OpImageGather %v4float %184 %31 %int_2
               OpStore %c %185
        %186 = OpLoad %45 %texCubeArray
        %187 = OpLoad %12 %texCubeArraySamp
        %188 = OpSampledImage %46 %186 %187
        %189 = OpImageGather %v4float %188 %50 %int_3
               OpStore %c %189
        %190 = OpLoad %52 %depth2d
        %191 = OpLoad %12 %depth2dSamp
        %192 = OpSampledImage %53 %190 %191
        %193 = OpImageDrefGather %v4float %192 %23 %float_1
               OpStore %c %193
        %194 = OpLoad %65 %depthCube
        %195 = OpLoad %12 %depthCubeSamp
        %196 = OpSampledImage %66 %194 %195
        %197 = OpImageDrefGather %v4float %196 %31 %float_1
               OpStore %c %197
        %198 = OpLoad %74 %depth2dArray
        %199 = OpLoad %12 %depth2dArraySamp
        %200 = OpSampledImage %75 %198 %199
        %201 = OpImageDrefGather %v4float %200 %31 %float_1
               OpStore %c %201
        %202 = OpLoad %82 %depthCubeArray
        %203 = OpLoad %12 %depthCubeArraySamp
        %204 = OpSampledImage %83 %202 %203
        %205 = OpImageDrefGather %v4float %204 %50 %float_1
               OpStore %c %205
               OpReturn
               OpFunctionEnd
