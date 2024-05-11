; SPIR-V
; Version: 1.3
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 325
; Schema: 0
               OpCapability Shader
               OpCapability SampleRateShading
               OpCapability InterpolationFunction
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %baz %a %s %foo %sid %bar %b %c
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %func_ "func("
               OpName %FragColor "FragColor"
               OpName %baz "baz"
               OpName %a "a"
               OpName %_ ""
               OpMemberName %_ 0 "x"
               OpMemberName %_ 1 "y"
               OpMemberName %_ 2 "z"
               OpMemberName %_ 3 "u"
               OpMemberName %_ 4 "v"
               OpMemberName %_ 5 "w"
               OpName %s "s"
               OpName %foo "foo"
               OpName %sid "sid"
               OpName %bar "bar"
               OpName %b "b"
               OpName %c "c"
               OpDecorate %FragColor Location 0
               OpDecorate %baz Sample
               OpDecorate %baz Location 2
               OpDecorate %a Location 4
               OpDecorate %s Location 10
               OpDecorate %foo NoPerspective
               OpDecorate %foo Location 0
               OpDecorate %sid Flat
               OpDecorate %sid Location 3
               OpDecorate %bar Centroid
               OpDecorate %bar Location 1
               OpDecorate %b Centroid
               OpDecorate %b Location 6
               OpDecorate %c Sample
               OpDecorate %c Location 8
               OpMemberDecorate %_ 1 Centroid
               OpMemberDecorate %_ 1 NoPerspective
               OpMemberDecorate %_ 2 Sample
               OpMemberDecorate %_ 3 Centroid
               OpMemberDecorate %_ 4 Sample
               OpMemberDecorate %_ 4 NoPerspective
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %baz = OpVariable %_ptr_Input_v2float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_float = OpTypePointer Output %float
     %uint_1 = OpConstant %uint 1
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
%float_n0_100000001 = OpConstant %float -0.100000001
%float_0_100000001 = OpConstant %float 0.100000001
         %30 = OpConstantComposite %v2float %float_n0_100000001 %float_0_100000001
     %uint_2 = OpConstant %uint 2
%_arr_v2float_uint_2 = OpTypeArray %v2float %uint_2
%_ptr_Input__arr_v2float_uint_2 = OpTypePointer Input %_arr_v2float_uint_2
          %a = OpVariable %_ptr_Input__arr_v2float_uint_2 Input
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_ptr_Input__arr_v4float_uint_2 = OpTypePointer Input %_arr_v4float_uint_2
     %uint_3 = OpConstant %uint 3
%_arr_float_uint_3 = OpTypeArray %float %uint_3
          %_ = OpTypeStruct %v4float %v4float %v4float %_arr_v4float_uint_2 %_arr_v2float_uint_2 %_arr_float_uint_3
%_ptr_Input__ = OpTypePointer Input %_
          %s = OpVariable %_ptr_Input__ Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
        %foo = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_int = OpTypePointer Input %int
        %sid = OpVariable %_ptr_Input_int Input
         %44 = OpConstantComposite %v2float %float_0_100000001 %float_0_100000001
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
        %bar = OpVariable %_ptr_Input_v3float Input
         %47 = OpConstantComposite %v2float %float_n0_100000001 %float_n0_100000001
          %b = OpVariable %_ptr_Input__arr_v2float_uint_2 Input
          %c = OpVariable %_ptr_Input__arr_v2float_uint_2 Input
      %int_4 = OpConstant %int 4
      %int_5 = OpConstant %int 5
       %main = OpFunction %void None %15
         %50 = OpLabel
         %51 = OpLoad %v4float %foo
               OpStore %FragColor %51
         %52 = OpExtInst %v4float %1 InterpolateAtCentroid %foo
         %53 = OpLoad %v4float %FragColor
         %54 = OpFAdd %v4float %53 %52
               OpStore %FragColor %54
         %55 = OpLoad %int %sid
         %56 = OpExtInst %v4float %1 InterpolateAtSample %foo %55
         %57 = OpLoad %v4float %FragColor
         %58 = OpFAdd %v4float %57 %56
               OpStore %FragColor %58
         %59 = OpExtInst %v4float %1 InterpolateAtOffset %foo %44
         %60 = OpLoad %v4float %FragColor
         %61 = OpFAdd %v4float %60 %59
               OpStore %FragColor %61
         %62 = OpLoad %v3float %bar
         %63 = OpLoad %v4float %FragColor
         %64 = OpVectorShuffle %v3float %63 %63 0 1 2
         %65 = OpFAdd %v3float %64 %62
         %66 = OpLoad %v4float %FragColor
         %67 = OpVectorShuffle %v4float %66 %65 4 5 6 3
               OpStore %FragColor %67
         %68 = OpExtInst %v3float %1 InterpolateAtCentroid %bar
         %69 = OpLoad %v4float %FragColor
         %70 = OpVectorShuffle %v3float %69 %69 0 1 2
         %71 = OpFAdd %v3float %70 %68
         %72 = OpLoad %v4float %FragColor
         %73 = OpVectorShuffle %v4float %72 %71 4 5 6 3
               OpStore %FragColor %73
         %74 = OpLoad %int %sid
         %75 = OpExtInst %v3float %1 InterpolateAtSample %bar %74
         %76 = OpLoad %v4float %FragColor
         %77 = OpVectorShuffle %v3float %76 %76 0 1 2
         %78 = OpFAdd %v3float %77 %75
         %79 = OpLoad %v4float %FragColor
         %80 = OpVectorShuffle %v4float %79 %78 4 5 6 3
               OpStore %FragColor %80
         %81 = OpExtInst %v3float %1 InterpolateAtOffset %bar %47
         %82 = OpLoad %v4float %FragColor
         %83 = OpVectorShuffle %v3float %82 %82 0 1 2
         %84 = OpFAdd %v3float %83 %81
         %85 = OpLoad %v4float %FragColor
         %86 = OpVectorShuffle %v4float %85 %84 4 5 6 3
               OpStore %FragColor %86
         %87 = OpAccessChain %_ptr_Input_v2float %b %int_0
         %88 = OpLoad %v2float %87
         %89 = OpLoad %v4float %FragColor
         %90 = OpVectorShuffle %v2float %89 %89 0 1
         %91 = OpFAdd %v2float %90 %88
         %92 = OpLoad %v4float %FragColor
         %93 = OpVectorShuffle %v4float %92 %91 4 5 2 3
               OpStore %FragColor %93
         %94 = OpAccessChain %_ptr_Input_v2float %b %int_1
         %95 = OpExtInst %v2float %1 InterpolateAtCentroid %94
         %96 = OpLoad %v4float %FragColor
         %97 = OpVectorShuffle %v2float %96 %96 0 1
         %98 = OpFAdd %v2float %97 %95
         %99 = OpLoad %v4float %FragColor
        %100 = OpVectorShuffle %v4float %99 %98 4 5 2 3
               OpStore %FragColor %100
        %101 = OpAccessChain %_ptr_Input_v2float %b %int_0
        %102 = OpExtInst %v2float %1 InterpolateAtSample %101 %int_2
        %103 = OpLoad %v4float %FragColor
        %104 = OpVectorShuffle %v2float %103 %103 0 1
        %105 = OpFAdd %v2float %104 %102
        %106 = OpLoad %v4float %FragColor
        %107 = OpVectorShuffle %v4float %106 %105 4 5 2 3
               OpStore %FragColor %107
        %108 = OpAccessChain %_ptr_Input_v2float %b %int_1
        %109 = OpExtInst %v2float %1 InterpolateAtOffset %108 %30
        %110 = OpLoad %v4float %FragColor
        %111 = OpVectorShuffle %v2float %110 %110 0 1
        %112 = OpFAdd %v2float %111 %109
        %113 = OpLoad %v4float %FragColor
        %114 = OpVectorShuffle %v4float %113 %112 4 5 2 3
               OpStore %FragColor %114
        %115 = OpAccessChain %_ptr_Input_v2float %c %int_0
        %116 = OpLoad %v2float %115
        %117 = OpLoad %v4float %FragColor
        %118 = OpVectorShuffle %v2float %117 %117 0 1
        %119 = OpFAdd %v2float %118 %116
        %120 = OpLoad %v4float %FragColor
        %121 = OpVectorShuffle %v4float %120 %119 4 5 2 3
               OpStore %FragColor %121
        %122 = OpAccessChain %_ptr_Input_v2float %c %int_1
        %123 = OpExtInst %v2float %1 InterpolateAtCentroid %122
        %124 = OpVectorShuffle %v2float %123 %123 0 1
        %125 = OpLoad %v4float %FragColor
        %126 = OpVectorShuffle %v2float %125 %125 0 1
        %127 = OpFAdd %v2float %126 %124
        %128 = OpLoad %v4float %FragColor
        %129 = OpVectorShuffle %v4float %128 %127 4 5 2 3
               OpStore %FragColor %129
        %130 = OpAccessChain %_ptr_Input_v2float %c %int_0
        %131 = OpExtInst %v2float %1 InterpolateAtSample %130 %int_2
        %132 = OpVectorShuffle %v2float %131 %131 1 0
        %133 = OpLoad %v4float %FragColor
        %134 = OpVectorShuffle %v2float %133 %133 0 1
        %135 = OpFAdd %v2float %134 %132
        %136 = OpLoad %v4float %FragColor
        %137 = OpVectorShuffle %v4float %136 %135 4 5 2 3
               OpStore %FragColor %137
        %138 = OpAccessChain %_ptr_Input_v2float %c %int_1
        %139 = OpExtInst %v2float %1 InterpolateAtOffset %138 %30
        %140 = OpVectorShuffle %v2float %139 %139 0 0
        %141 = OpLoad %v4float %FragColor
        %142 = OpVectorShuffle %v2float %141 %141 0 1
        %143 = OpFAdd %v2float %142 %140
        %144 = OpLoad %v4float %FragColor
        %145 = OpVectorShuffle %v4float %144 %143 4 5 2 3
               OpStore %FragColor %145
        %146 = OpAccessChain %_ptr_Input_v4float %s %int_0
        %147 = OpLoad %v4float %146
        %148 = OpLoad %v4float %FragColor
        %149 = OpFAdd %v4float %148 %147
               OpStore %FragColor %149
        %150 = OpAccessChain %_ptr_Input_v4float %s %int_0
        %151 = OpExtInst %v4float %1 InterpolateAtCentroid %150
        %152 = OpLoad %v4float %FragColor
        %153 = OpFAdd %v4float %152 %151
               OpStore %FragColor %153
        %154 = OpAccessChain %_ptr_Input_v4float %s %int_0
        %155 = OpLoad %int %sid
        %156 = OpExtInst %v4float %1 InterpolateAtSample %154 %155
        %157 = OpLoad %v4float %FragColor
        %158 = OpFAdd %v4float %157 %156
               OpStore %FragColor %158
        %159 = OpAccessChain %_ptr_Input_v4float %s %int_0
        %160 = OpExtInst %v4float %1 InterpolateAtOffset %159 %44
        %161 = OpLoad %v4float %FragColor
        %162 = OpFAdd %v4float %161 %160
               OpStore %FragColor %162
        %163 = OpAccessChain %_ptr_Input_v4float %s %int_1
        %164 = OpLoad %v4float %163
        %165 = OpLoad %v4float %FragColor
        %166 = OpFAdd %v4float %165 %164
               OpStore %FragColor %166
        %167 = OpAccessChain %_ptr_Input_v4float %s %int_1
        %168 = OpExtInst %v4float %1 InterpolateAtCentroid %167
        %169 = OpLoad %v4float %FragColor
        %170 = OpFAdd %v4float %169 %168
               OpStore %FragColor %170
        %171 = OpAccessChain %_ptr_Input_v4float %s %int_1
        %172 = OpLoad %int %sid
        %173 = OpExtInst %v4float %1 InterpolateAtSample %171 %172
        %174 = OpLoad %v4float %FragColor
        %175 = OpFAdd %v4float %174 %173
               OpStore %FragColor %175
        %176 = OpAccessChain %_ptr_Input_v4float %s %int_1
        %177 = OpExtInst %v4float %1 InterpolateAtOffset %176 %47
        %178 = OpLoad %v4float %FragColor
        %179 = OpFAdd %v4float %178 %177
               OpStore %FragColor %179
        %180 = OpAccessChain %_ptr_Input_v2float %s %int_4 %int_0
        %181 = OpLoad %v2float %180
        %182 = OpLoad %v4float %FragColor
        %183 = OpVectorShuffle %v2float %182 %182 0 1
        %184 = OpFAdd %v2float %183 %181
        %185 = OpLoad %v4float %FragColor
        %186 = OpVectorShuffle %v4float %185 %184 4 5 2 3
               OpStore %FragColor %186
        %187 = OpAccessChain %_ptr_Input_v2float %s %int_4 %int_1
        %188 = OpExtInst %v2float %1 InterpolateAtCentroid %187
        %189 = OpLoad %v4float %FragColor
        %190 = OpVectorShuffle %v2float %189 %189 0 1
        %191 = OpFAdd %v2float %190 %188
        %192 = OpLoad %v4float %FragColor
        %193 = OpVectorShuffle %v4float %192 %191 4 5 2 3
               OpStore %FragColor %193
        %194 = OpAccessChain %_ptr_Input_v2float %s %int_4 %int_0
        %195 = OpExtInst %v2float %1 InterpolateAtSample %194 %int_2
        %196 = OpLoad %v4float %FragColor
        %197 = OpVectorShuffle %v2float %196 %196 0 1
        %198 = OpFAdd %v2float %197 %195
        %199 = OpLoad %v4float %FragColor
        %200 = OpVectorShuffle %v4float %199 %198 4 5 2 3
               OpStore %FragColor %200
        %201 = OpAccessChain %_ptr_Input_v2float %s %int_4 %int_1
        %202 = OpExtInst %v2float %1 InterpolateAtOffset %201 %30
        %203 = OpLoad %v4float %FragColor
        %204 = OpVectorShuffle %v2float %203 %203 0 1
        %205 = OpFAdd %v2float %204 %202
        %206 = OpLoad %v4float %FragColor
        %207 = OpVectorShuffle %v4float %206 %205 4 5 2 3
               OpStore %FragColor %207
        %208 = OpAccessChain %_ptr_Input_float %s %int_5 %int_0
        %209 = OpLoad %float %208
        %210 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
        %211 = OpLoad %float %210
        %212 = OpFAdd %float %211 %209
        %213 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
               OpStore %213 %212
        %214 = OpAccessChain %_ptr_Input_float %s %int_5 %int_1
        %215 = OpExtInst %float %1 InterpolateAtCentroid %214
        %216 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
        %217 = OpLoad %float %216
        %218 = OpFAdd %float %217 %215
        %219 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
               OpStore %219 %218
        %220 = OpAccessChain %_ptr_Input_float %s %int_5 %int_0
        %221 = OpExtInst %float %1 InterpolateAtSample %220 %int_2
        %222 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
        %223 = OpLoad %float %222
        %224 = OpFAdd %float %223 %221
        %225 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
               OpStore %225 %224
        %226 = OpAccessChain %_ptr_Input_float %s %int_5 %int_1
        %227 = OpExtInst %float %1 InterpolateAtOffset %226 %30
        %228 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
        %229 = OpLoad %float %228
        %230 = OpFAdd %float %229 %227
        %231 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
               OpStore %231 %230
        %232 = OpFunctionCall %void %func_
               OpReturn
               OpFunctionEnd
      %func_ = OpFunction %void None %15
        %233 = OpLabel
        %234 = OpLoad %v2float %baz
        %235 = OpLoad %v4float %FragColor
        %236 = OpVectorShuffle %v2float %235 %235 0 1
        %237 = OpFAdd %v2float %236 %234
        %238 = OpLoad %v4float %FragColor
        %239 = OpVectorShuffle %v4float %238 %237 4 5 2 3
               OpStore %FragColor %239
        %240 = OpAccessChain %_ptr_Input_float %baz %uint_0
        %241 = OpExtInst %float %1 InterpolateAtCentroid %240
        %242 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
        %243 = OpLoad %float %242
        %244 = OpFAdd %float %243 %241
        %245 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
               OpStore %245 %244
        %246 = OpAccessChain %_ptr_Input_float %baz %uint_1
        %247 = OpExtInst %float %1 InterpolateAtSample %246 %int_3
        %248 = OpAccessChain %_ptr_Output_float %FragColor %uint_1
        %249 = OpLoad %float %248
        %250 = OpFAdd %float %249 %247
        %251 = OpAccessChain %_ptr_Output_float %FragColor %uint_1
               OpStore %251 %250
        %252 = OpAccessChain %_ptr_Input_float %baz %uint_1
        %253 = OpExtInst %float %1 InterpolateAtOffset %252 %30
        %254 = OpAccessChain %_ptr_Output_float %FragColor %uint_2
        %255 = OpLoad %float %254
        %256 = OpFAdd %float %255 %253
        %257 = OpAccessChain %_ptr_Output_float %FragColor %uint_2
               OpStore %257 %256
        %258 = OpAccessChain %_ptr_Input_v2float %a %int_1
        %259 = OpExtInst %v2float %1 InterpolateAtCentroid %258
        %260 = OpLoad %v4float %FragColor
        %261 = OpVectorShuffle %v2float %260 %260 0 1
        %262 = OpFAdd %v2float %261 %259
        %263 = OpLoad %v4float %FragColor
        %264 = OpVectorShuffle %v4float %263 %262 4 5 2 3
               OpStore %FragColor %264
        %265 = OpAccessChain %_ptr_Input_v2float %a %int_0
        %266 = OpExtInst %v2float %1 InterpolateAtSample %265 %int_2
        %267 = OpLoad %v4float %FragColor
        %268 = OpVectorShuffle %v2float %267 %267 0 1
        %269 = OpFAdd %v2float %268 %266
        %270 = OpLoad %v4float %FragColor
        %271 = OpVectorShuffle %v4float %270 %269 4 5 2 3
               OpStore %FragColor %271
        %272 = OpAccessChain %_ptr_Input_v2float %a %int_1
        %273 = OpExtInst %v2float %1 InterpolateAtOffset %272 %30
        %274 = OpLoad %v4float %FragColor
        %275 = OpVectorShuffle %v2float %274 %274 0 1
        %276 = OpFAdd %v2float %275 %273
        %277 = OpLoad %v4float %FragColor
        %278 = OpVectorShuffle %v4float %277 %276 4 5 2 3
               OpStore %FragColor %278
        %279 = OpAccessChain %_ptr_Input_v4float %s %int_2
        %280 = OpLoad %v4float %279
        %281 = OpLoad %v4float %FragColor
        %282 = OpFAdd %v4float %281 %280
               OpStore %FragColor %282
        %283 = OpAccessChain %_ptr_Input_v4float %s %int_2
        %284 = OpExtInst %v4float %1 InterpolateAtCentroid %283
        %285 = OpVectorShuffle %v2float %284 %284 1 1
        %286 = OpLoad %v4float %FragColor
        %287 = OpVectorShuffle %v2float %286 %286 0 1
        %288 = OpFAdd %v2float %287 %285
        %289 = OpLoad %v4float %FragColor
        %290 = OpVectorShuffle %v4float %289 %288 4 5 2 3
               OpStore %FragColor %290
        %291 = OpAccessChain %_ptr_Input_v4float %s %int_2
        %292 = OpExtInst %v4float %1 InterpolateAtSample %291 %int_3
        %293 = OpVectorShuffle %v2float %292 %292 0 1
        %294 = OpLoad %v4float %FragColor
        %295 = OpVectorShuffle %v2float %294 %294 1 2
        %296 = OpFAdd %v2float %295 %293
        %297 = OpLoad %v4float %FragColor
        %298 = OpVectorShuffle %v4float %297 %296 0 4 5 3
               OpStore %FragColor %298
        %299 = OpAccessChain %_ptr_Input_v4float %s %int_2
        %300 = OpExtInst %v4float %1 InterpolateAtOffset %299 %30
        %301 = OpVectorShuffle %v2float %300 %300 3 0
        %302 = OpLoad %v4float %FragColor
        %303 = OpVectorShuffle %v2float %302 %302 2 3
        %304 = OpFAdd %v2float %303 %301
        %305 = OpLoad %v4float %FragColor
        %306 = OpVectorShuffle %v4float %305 %304 0 1 4 5
               OpStore %FragColor %306
        %308 = OpAccessChain %_ptr_Input_v4float %s %int_3 %int_0
        %309 = OpLoad %v4float %308
        %310 = OpLoad %v4float %FragColor
        %311 = OpFAdd %v4float %310 %309
               OpStore %FragColor %311
        %312 = OpAccessChain %_ptr_Input__arr_v4float_uint_2 %s %int_3
        %313 = OpAccessChain %_ptr_Input_v4float %312 %int_1
        %314 = OpExtInst %v4float %1 InterpolateAtCentroid %313
        %315 = OpLoad %v4float %FragColor
        %316 = OpFAdd %v4float %315 %314
               OpStore %FragColor %316
        %317 = OpAccessChain %_ptr_Input_v4float %s %int_3 %int_0
        %318 = OpExtInst %v4float %1 InterpolateAtSample %317 %int_2
        %319 = OpLoad %v4float %FragColor
        %320 = OpFAdd %v4float %319 %318
               OpStore %FragColor %320
        %321 = OpAccessChain %_ptr_Input_v4float %s %int_3 %int_1
        %322 = OpExtInst %v4float %1 InterpolateAtOffset %321 %30
        %323 = OpLoad %v4float %FragColor
        %324 = OpFAdd %v4float %323 %322
               OpStore %FragColor %324
               OpReturn
               OpFunctionEnd
