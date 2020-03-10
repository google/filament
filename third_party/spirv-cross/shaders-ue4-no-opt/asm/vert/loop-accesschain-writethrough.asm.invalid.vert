; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 181
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %ScatterMainVS "main" %gl_VertexIndex %gl_InstanceIndex %out_var_TEXCOORD0 %out_var_TEXCOORD1 %out_var_TEXCOORD2 %out_var_TEXCOORD3 %out_var_TEXCOORD4 %out_var_TEXCOORD5 %out_var_TEXCOORD6 %gl_Position
               OpSource HLSL 600
               OpName %type__Globals "type.$Globals"
               OpMemberName %type__Globals 0 "ViewportSize"
               OpMemberName %type__Globals 1 "ScatteringScaling"
               OpMemberName %type__Globals 2 "CocRadiusToCircumscribedRadius"
               OpName %_Globals "$Globals"
               OpName %type_StructuredBuffer_v4float "type.StructuredBuffer.v4float"
               OpName %ScatterDrawList "ScatterDrawList"
               OpName %out_var_TEXCOORD0 "out.var.TEXCOORD0"
               OpName %out_var_TEXCOORD1 "out.var.TEXCOORD1"
               OpName %out_var_TEXCOORD2 "out.var.TEXCOORD2"
               OpName %out_var_TEXCOORD3 "out.var.TEXCOORD3"
               OpName %out_var_TEXCOORD4 "out.var.TEXCOORD4"
               OpName %out_var_TEXCOORD5 "out.var.TEXCOORD5"
               OpName %out_var_TEXCOORD6 "out.var.TEXCOORD6"
               OpName %ScatterMainVS "ScatterMainVS"
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
               OpDecorateString %gl_VertexIndex UserSemantic "SV_VertexID"
               OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
               OpDecorateString %gl_InstanceIndex UserSemantic "SV_InstanceID"
               OpDecorateString %out_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorateString %out_var_TEXCOORD1 UserSemantic "TEXCOORD1"
               OpDecorateString %out_var_TEXCOORD2 UserSemantic "TEXCOORD2"
               OpDecorateString %out_var_TEXCOORD3 UserSemantic "TEXCOORD3"
               OpDecorateString %out_var_TEXCOORD4 UserSemantic "TEXCOORD4"
               OpDecorateString %out_var_TEXCOORD5 UserSemantic "TEXCOORD5"
               OpDecorateString %out_var_TEXCOORD6 UserSemantic "TEXCOORD6"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorateString %gl_Position UserSemantic "SV_POSITION"
               OpDecorate %out_var_TEXCOORD0 Location 0
               OpDecorate %out_var_TEXCOORD1 Location 1
               OpDecorate %out_var_TEXCOORD2 Location 2
               OpDecorate %out_var_TEXCOORD3 Location 3
               OpDecorate %out_var_TEXCOORD4 Location 4
               OpDecorate %out_var_TEXCOORD5 Location 5
               OpDecorate %out_var_TEXCOORD6 Location 6
               OpDecorate %_Globals DescriptorSet 0
               OpDecorate %_Globals Binding 1
               OpDecorate %ScatterDrawList DescriptorSet 0
               OpDecorate %ScatterDrawList Binding 0
               OpMemberDecorate %type__Globals 0 Offset 0
               OpMemberDecorate %type__Globals 1 Offset 16
               OpMemberDecorate %type__Globals 2 Offset 20
               OpDecorate %type__Globals Block
               OpDecorate %_runtimearr_v4float ArrayStride 16
               OpMemberDecorate %type_StructuredBuffer_v4float 0 Offset 0
               OpMemberDecorate %type_StructuredBuffer_v4float 0 NonWritable
               OpDecorate %type_StructuredBuffer_v4float BufferBlock
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_4 = OpConstant %uint 4
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
  %float_0_5 = OpConstant %float 0.5
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
    %uint_16 = OpConstant %uint 16
    %float_0 = OpConstant %float 0
     %uint_0 = OpConstant %uint 0
     %uint_5 = OpConstant %uint 5
     %uint_1 = OpConstant %uint 1
      %int_3 = OpConstant %int 3
 %float_n0_5 = OpConstant %float -0.5
      %int_2 = OpConstant %int 2
    %float_2 = OpConstant %float 2
         %39 = OpConstantComposite %v2float %float_2 %float_2
         %40 = OpConstantComposite %v2float %float_1 %float_1
         %41 = OpConstantComposite %v2float %float_0_5 %float_0_5
%type__Globals = OpTypeStruct %v4float %float %float
%_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
%type_StructuredBuffer_v4float = OpTypeStruct %_runtimearr_v4float
%_ptr_Uniform_type_StructuredBuffer_v4float = OpTypePointer Uniform %type_StructuredBuffer_v4float
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Output_v2float = OpTypePointer Output %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %48 = OpTypeFunction %void
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function__arr_v4float_uint_4 = OpTypePointer Function %_arr_v4float_uint_4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Function__arr_float_uint_4 = OpTypePointer Function %_arr_float_uint_4
%_arr_v2float_uint_4 = OpTypeArray %v2float %uint_4
%_ptr_Function__arr_v2float_uint_4 = OpTypePointer Function %_arr_v2float_uint_4
%_ptr_Function_float = OpTypePointer Function %float
       %bool = OpTypeBool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
   %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
%ScatterDrawList = OpVariable %_ptr_Uniform_type_StructuredBuffer_v4float Uniform
%gl_VertexIndex = OpVariable %_ptr_Input_uint Input
%gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
%out_var_TEXCOORD0 = OpVariable %_ptr_Output_v2float Output
%out_var_TEXCOORD1 = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD2 = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD3 = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD4 = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD5 = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD6 = OpVariable %_ptr_Output_v4float Output
%gl_Position = OpVariable %_ptr_Output_v4float Output
%ScatterMainVS = OpFunction %void None %48
         %60 = OpLabel
         %61 = OpVariable %_ptr_Function__arr_v4float_uint_4 Function
         %62 = OpVariable %_ptr_Function__arr_float_uint_4 Function
         %63 = OpVariable %_ptr_Function__arr_v2float_uint_4 Function
         %64 = OpLoad %uint %gl_VertexIndex
         %65 = OpLoad %uint %gl_InstanceIndex
         %66 = OpUDiv %uint %64 %uint_4
         %67 = OpIMul %uint %66 %uint_4
         %68 = OpISub %uint %64 %67
         %69 = OpIMul %uint %uint_16 %65
         %70 = OpIAdd %uint %69 %66
               OpBranch %71
         %71 = OpLabel
         %72 = OpPhi %float %float_0 %60 %73 %74
         %75 = OpPhi %uint %uint_0 %60 %76 %74
         %77 = OpULessThan %bool %75 %uint_4
               OpLoopMerge %78 %74 Unroll
               OpBranchConditional %77 %79 %78
         %79 = OpLabel
         %80 = OpIMul %uint %uint_5 %70
         %81 = OpIAdd %uint %80 %75
         %82 = OpIAdd %uint %81 %uint_1
         %83 = OpAccessChain %_ptr_Uniform_v4float %ScatterDrawList %int_0 %82
         %84 = OpLoad %v4float %83
         %85 = OpCompositeExtract %float %84 0
         %86 = OpCompositeExtract %float %84 1
         %87 = OpCompositeExtract %float %84 2
         %88 = OpCompositeConstruct %v4float %85 %86 %87 %float_0
         %89 = OpAccessChain %_ptr_Function_v4float %61 %75
               OpStore %89 %88
         %90 = OpCompositeExtract %float %84 3
         %91 = OpAccessChain %_ptr_Function_float %62 %75
               OpStore %91 %90
         %92 = OpIEqual %bool %75 %uint_0
               OpSelectionMerge %74 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
         %95 = OpLoad %float %91
               OpBranch %74
         %94 = OpLabel
         %96 = OpLoad %float %91
         %97 = OpExtInst %float %1 FMax %72 %96
               OpBranch %74
         %74 = OpLabel
         %73 = OpPhi %float %95 %93 %97 %94
         %98 = OpLoad %float %91
         %99 = OpFDiv %float %float_n0_5 %98
        %100 = OpAccessChain %_ptr_Function_float %63 %75 %int_0
               OpStore %100 %99
        %101 = OpLoad %float %91
        %102 = OpFMul %float %float_0_5 %101
        %103 = OpFAdd %float %102 %float_0_5
        %104 = OpAccessChain %_ptr_Function_float %63 %75 %int_1
               OpStore %104 %103
         %76 = OpIAdd %uint %75 %uint_1
               OpBranch %71
         %78 = OpLabel
        %105 = OpAccessChain %_ptr_Function_v4float %61 %int_0
        %106 = OpLoad %v4float %105
        %107 = OpCompositeExtract %float %106 0
        %108 = OpCompositeExtract %float %106 1
        %109 = OpCompositeExtract %float %106 2
        %110 = OpAccessChain %_ptr_Function_float %62 %int_0
        %111 = OpLoad %float %110
        %112 = OpCompositeConstruct %v4float %107 %108 %109 %111
        %113 = OpAccessChain %_ptr_Function_v4float %61 %int_1
        %114 = OpLoad %v4float %113
        %115 = OpCompositeExtract %float %114 0
        %116 = OpCompositeExtract %float %114 1
        %117 = OpCompositeExtract %float %114 2
        %118 = OpAccessChain %_ptr_Function_float %62 %int_1
        %119 = OpLoad %float %118
        %120 = OpCompositeConstruct %v4float %115 %116 %117 %119
        %121 = OpAccessChain %_ptr_Function_v4float %61 %int_2
        %122 = OpLoad %v4float %121
        %123 = OpCompositeExtract %float %122 0
        %124 = OpCompositeExtract %float %122 1
        %125 = OpCompositeExtract %float %122 2
        %126 = OpAccessChain %_ptr_Function_float %62 %int_2
        %127 = OpLoad %float %126
        %128 = OpCompositeConstruct %v4float %123 %124 %125 %127
        %129 = OpAccessChain %_ptr_Function_v4float %61 %int_3
        %130 = OpLoad %v4float %129
        %131 = OpCompositeExtract %float %130 0
        %132 = OpCompositeExtract %float %130 1
        %133 = OpCompositeExtract %float %130 2
        %134 = OpAccessChain %_ptr_Function_float %62 %int_3
        %135 = OpLoad %float %134
        %136 = OpCompositeConstruct %v4float %131 %132 %133 %135
        %137 = OpAccessChain %_ptr_Uniform_float %_Globals %int_1
        %138 = OpLoad %float %137
        %139 = OpCompositeConstruct %v2float %138 %138
        %140 = OpIMul %uint %uint_5 %70
        %141 = OpAccessChain %_ptr_Uniform_v4float %ScatterDrawList %int_0 %140
        %142 = OpLoad %v4float %141
        %143 = OpVectorShuffle %v2float %142 %142 0 1
        %144 = OpFMul %v2float %139 %143
        %145 = OpAccessChain %_ptr_Function_v2float %63 %int_0
        %146 = OpLoad %v2float %145
        %147 = OpAccessChain %_ptr_Function_v2float %63 %int_1
        %148 = OpLoad %v2float %147
        %149 = OpVectorShuffle %v4float %146 %148 0 1 2 3
        %150 = OpAccessChain %_ptr_Function_v2float %63 %int_2
        %151 = OpLoad %v2float %150
        %152 = OpAccessChain %_ptr_Function_v2float %63 %int_3
        %153 = OpLoad %v2float %152
        %154 = OpVectorShuffle %v4float %151 %153 0 1 2 3
        %155 = OpUMod %uint %68 %uint_2
        %156 = OpConvertUToF %float %155
        %157 = OpUDiv %uint %68 %uint_2
        %158 = OpConvertUToF %float %157
        %159 = OpCompositeConstruct %v2float %156 %158
        %160 = OpFMul %v2float %159 %39
        %161 = OpFSub %v2float %160 %40
        %162 = OpAccessChain %_ptr_Uniform_float %_Globals %int_2
        %163 = OpLoad %float %162
        %164 = OpFMul %float %72 %163
        %165 = OpFAdd %float %164 %float_1
        %166 = OpCompositeConstruct %v2float %165 %165
        %167 = OpFMul %v2float %166 %161
        %168 = OpFAdd %v2float %167 %144
        %169 = OpFAdd %v2float %168 %41
        %170 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_0
        %171 = OpLoad %v4float %170
        %172 = OpVectorShuffle %v2float %171 %171 2 3
        %173 = OpFMul %v2float %169 %172
        %174 = OpCompositeExtract %float %173 0
        %175 = OpFMul %float %174 %float_2
        %176 = OpFSub %float %175 %float_1
        %177 = OpCompositeExtract %float %173 1
        %178 = OpFMul %float %177 %float_2
        %179 = OpFSub %float %float_1 %178
        %180 = OpCompositeConstruct %v4float %176 %179 %float_0 %float_1
               OpStore %out_var_TEXCOORD0 %144
               OpStore %out_var_TEXCOORD1 %112
               OpStore %out_var_TEXCOORD2 %120
               OpStore %out_var_TEXCOORD3 %128
               OpStore %out_var_TEXCOORD4 %136
               OpStore %out_var_TEXCOORD5 %149
               OpStore %out_var_TEXCOORD6 %154
               OpStore %gl_Position %180
               OpReturn
               OpFunctionEnd
