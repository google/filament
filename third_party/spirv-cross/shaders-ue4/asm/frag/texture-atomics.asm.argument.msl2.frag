; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 180
; Schema: 0
               OpCapability Shader
               OpCapability SampledBuffer
               OpCapability ImageBuffer
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %ShadowObjectCullPS "main" %in_var_TEXCOORD0 %gl_FragCoord %out_var_SV_Target0
               OpExecutionMode %ShadowObjectCullPS OriginUpperLeft
               OpSource HLSL 600
               OpName %type_StructuredBuffer_v4float "type.StructuredBuffer.v4float"
               OpName %CulledObjectBoxBounds "CulledObjectBoxBounds"
               OpName %type__Globals "type.$Globals"
               OpMemberName %type__Globals 0 "ShadowTileListGroupSize"
               OpName %_Globals "$Globals"
               OpName %type_buffer_image "type.buffer.image"
               OpName %RWShadowTileNumCulledObjects "RWShadowTileNumCulledObjects"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %ShadowObjectCullPS "ShadowObjectCullPS"
               OpDecorateString %in_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorate %in_var_TEXCOORD0 Flat
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorateString %gl_FragCoord UserSemantic "SV_POSITION"
               OpDecorateString %out_var_SV_Target0 UserSemantic "SV_Target0"
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %CulledObjectBoxBounds DescriptorSet 0
               OpDecorate %CulledObjectBoxBounds Binding 1
               OpDecorate %_Globals DescriptorSet 0
               OpDecorate %_Globals Binding 2
               OpDecorate %RWShadowTileNumCulledObjects DescriptorSet 0
               OpDecorate %RWShadowTileNumCulledObjects Binding 0
               OpDecorate %_runtimearr_v4float ArrayStride 16
               OpMemberDecorate %type_StructuredBuffer_v4float 0 Offset 0
               OpMemberDecorate %type_StructuredBuffer_v4float 0 NonWritable
               OpDecorate %type_StructuredBuffer_v4float BufferBlock
               OpMemberDecorate %type__Globals 0 Offset 0
               OpDecorate %type__Globals Block
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_4 = OpConstant %uint 4
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
    %float_2 = OpConstant %float 2
         %27 = OpConstantComposite %v2float %float_2 %float_2
    %float_1 = OpConstant %float 1
         %29 = OpConstantComposite %v2float %float_1 %float_1
%float_n1000 = OpConstant %float -1000
      %int_2 = OpConstant %int 2
  %float_0_5 = OpConstant %float 0.5
         %33 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%float_500000 = OpConstant %float 500000
         %35 = OpConstantComposite %v3float %float_500000 %float_500000 %float_500000
%float_n500000 = OpConstant %float -500000
         %37 = OpConstantComposite %v3float %float_n500000 %float_n500000 %float_n500000
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
      %int_5 = OpConstant %int 5
      %int_6 = OpConstant %int 6
      %int_7 = OpConstant %int 7
      %int_8 = OpConstant %int 8
         %44 = OpConstantComposite %v3float %float_1 %float_1 %float_1
   %float_n1 = OpConstant %float -1
         %46 = OpConstantComposite %v3float %float_n1 %float_n1 %float_n1
     %uint_5 = OpConstant %uint 5
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
%type_StructuredBuffer_v4float = OpTypeStruct %_runtimearr_v4float
%_ptr_Uniform_type_StructuredBuffer_v4float = OpTypePointer Uniform %type_StructuredBuffer_v4float
     %v2uint = OpTypeVector %uint 2
%type__Globals = OpTypeStruct %v2uint
%_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals
%type_buffer_image = OpTypeImage %uint Buffer 2 0 0 2 R32ui
%_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %58 = OpTypeFunction %void
%_ptr_Function_v3float = OpTypePointer Function %v3float
     %uint_8 = OpConstant %uint 8
%_arr_v3float_uint_8 = OpTypeArray %v3float %uint_8
%_ptr_Function__arr_v3float_uint_8 = OpTypePointer Function %_arr_v3float_uint_8
%_ptr_Uniform_v2uint = OpTypePointer Uniform %v2uint
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Image_uint = OpTypePointer Image %uint
%CulledObjectBoxBounds = OpVariable %_ptr_Uniform_type_StructuredBuffer_v4float Uniform
   %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
%RWShadowTileNumCulledObjects = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_uint Input
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %70 = OpUndef %v3float
         %71 = OpConstantNull %v3float
%ShadowObjectCullPS = OpFunction %void None %58
         %72 = OpLabel
         %73 = OpVariable %_ptr_Function__arr_v3float_uint_8 Function
         %74 = OpLoad %uint %in_var_TEXCOORD0
         %75 = OpLoad %v4float %gl_FragCoord
         %76 = OpVectorShuffle %v2float %75 %75 0 1
         %77 = OpConvertFToU %v2uint %76
         %78 = OpCompositeExtract %uint %77 1
         %79 = OpAccessChain %_ptr_Uniform_v2uint %_Globals %int_0
         %80 = OpAccessChain %_ptr_Uniform_uint %_Globals %int_0 %int_0
         %81 = OpLoad %uint %80
         %82 = OpIMul %uint %78 %81
         %83 = OpCompositeExtract %uint %77 0
         %84 = OpIAdd %uint %82 %83
         %85 = OpConvertUToF %float %83
         %86 = OpAccessChain %_ptr_Uniform_uint %_Globals %int_0 %int_1
         %87 = OpLoad %uint %86
         %88 = OpISub %uint %87 %uint_1
         %89 = OpISub %uint %88 %78
         %90 = OpConvertUToF %float %89
         %91 = OpCompositeConstruct %v2float %85 %90
         %92 = OpLoad %v2uint %79
         %93 = OpConvertUToF %v2float %92
         %94 = OpFDiv %v2float %91 %93
         %95 = OpFMul %v2float %94 %27
         %96 = OpFSub %v2float %95 %29
         %97 = OpFAdd %v2float %91 %29
         %98 = OpFDiv %v2float %97 %93
         %99 = OpFMul %v2float %98 %27
        %100 = OpFSub %v2float %99 %29
        %101 = OpVectorShuffle %v3float %70 %100 3 4 2
        %102 = OpCompositeInsert %v3float %float_1 %101 2
        %103 = OpIMul %uint %74 %uint_5
        %104 = OpAccessChain %_ptr_Uniform_v4float %CulledObjectBoxBounds %int_0 %103
        %105 = OpLoad %v4float %104
        %106 = OpVectorShuffle %v3float %105 %105 0 1 2
        %107 = OpIAdd %uint %103 %uint_1
        %108 = OpAccessChain %_ptr_Uniform_v4float %CulledObjectBoxBounds %int_0 %107
        %109 = OpLoad %v4float %108
        %110 = OpVectorShuffle %v3float %109 %109 0 1 2
        %111 = OpVectorShuffle %v2float %109 %71 0 1
        %112 = OpVectorShuffle %v2float %96 %71 0 1
        %113 = OpFOrdGreaterThan %v2bool %111 %112
        %114 = OpAll %bool %113
        %115 = OpFOrdLessThan %v3bool %106 %102
        %116 = OpAll %bool %115
        %117 = OpLogicalAnd %bool %114 %116
               OpSelectionMerge %118 DontFlatten
               OpBranchConditional %117 %119 %118
        %119 = OpLabel
        %120 = OpFAdd %v3float %106 %110
        %121 = OpFMul %v3float %33 %120
        %122 = OpCompositeExtract %float %96 0
        %123 = OpCompositeExtract %float %96 1
        %124 = OpCompositeConstruct %v3float %122 %123 %float_n1000
        %125 = OpAccessChain %_ptr_Function_v3float %73 %int_0
               OpStore %125 %124
        %126 = OpCompositeExtract %float %100 0
        %127 = OpCompositeConstruct %v3float %126 %123 %float_n1000
        %128 = OpAccessChain %_ptr_Function_v3float %73 %int_1
               OpStore %128 %127
        %129 = OpCompositeExtract %float %100 1
        %130 = OpCompositeConstruct %v3float %122 %129 %float_n1000
        %131 = OpAccessChain %_ptr_Function_v3float %73 %int_2
               OpStore %131 %130
        %132 = OpCompositeConstruct %v3float %126 %129 %float_n1000
        %133 = OpAccessChain %_ptr_Function_v3float %73 %int_3
               OpStore %133 %132
        %134 = OpCompositeConstruct %v3float %122 %123 %float_1
        %135 = OpAccessChain %_ptr_Function_v3float %73 %int_4
               OpStore %135 %134
        %136 = OpCompositeConstruct %v3float %126 %123 %float_1
        %137 = OpAccessChain %_ptr_Function_v3float %73 %int_5
               OpStore %137 %136
        %138 = OpCompositeConstruct %v3float %122 %129 %float_1
        %139 = OpAccessChain %_ptr_Function_v3float %73 %int_6
               OpStore %139 %138
        %140 = OpCompositeConstruct %v3float %126 %129 %float_1
        %141 = OpAccessChain %_ptr_Function_v3float %73 %int_7
               OpStore %141 %140
        %142 = OpIAdd %uint %103 %uint_2
        %143 = OpAccessChain %_ptr_Uniform_v4float %CulledObjectBoxBounds %int_0 %142
        %144 = OpLoad %v4float %143
        %145 = OpVectorShuffle %v3float %144 %144 0 1 2
        %146 = OpIAdd %uint %103 %uint_3
        %147 = OpAccessChain %_ptr_Uniform_v4float %CulledObjectBoxBounds %int_0 %146
        %148 = OpLoad %v4float %147
        %149 = OpVectorShuffle %v3float %148 %148 0 1 2
        %150 = OpIAdd %uint %103 %uint_4
        %151 = OpAccessChain %_ptr_Uniform_v4float %CulledObjectBoxBounds %int_0 %150
        %152 = OpLoad %v4float %151
        %153 = OpVectorShuffle %v3float %152 %152 0 1 2
               OpBranch %154
        %154 = OpLabel
        %155 = OpPhi %v3float %37 %119 %156 %157
        %158 = OpPhi %v3float %35 %119 %159 %157
        %160 = OpPhi %int %int_0 %119 %161 %157
        %162 = OpSLessThan %bool %160 %int_8
               OpLoopMerge %163 %157 Unroll
               OpBranchConditional %162 %157 %163
        %157 = OpLabel
        %164 = OpAccessChain %_ptr_Function_v3float %73 %160
        %165 = OpLoad %v3float %164
        %166 = OpFSub %v3float %165 %121
        %167 = OpDot %float %166 %145
        %168 = OpDot %float %166 %149
        %169 = OpDot %float %166 %153
        %170 = OpCompositeConstruct %v3float %167 %168 %169
        %159 = OpExtInst %v3float %1 FMin %158 %170
        %156 = OpExtInst %v3float %1 FMax %155 %170
        %161 = OpIAdd %int %160 %int_1
               OpBranch %154
        %163 = OpLabel
        %171 = OpFOrdLessThan %v3bool %158 %44
        %172 = OpAll %bool %171
        %173 = OpFOrdGreaterThan %v3bool %155 %46
        %174 = OpAll %bool %173
        %175 = OpLogicalAnd %bool %172 %174
               OpSelectionMerge %176 DontFlatten
               OpBranchConditional %175 %177 %176
        %177 = OpLabel
        %178 = OpImageTexelPointer %_ptr_Image_uint %RWShadowTileNumCulledObjects %84 %uint_0
        %179 = OpAtomicIAdd %uint %178 %uint_1 %uint_0 %uint_1
               OpBranch %176
        %176 = OpLabel
               OpBranch %118
        %118 = OpLabel
               OpStore %out_var_SV_Target0 %22
               OpReturn
               OpFunctionEnd
