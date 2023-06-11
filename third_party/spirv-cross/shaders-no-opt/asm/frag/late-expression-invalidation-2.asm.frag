; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 761
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_FragCoord %_GLF_color
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 320
               OpName %main "main"
               OpName %pos "pos"
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %buf0 "buf0"
               OpMemberName %buf0 0 "resolution"
               OpName %_ ""
               OpName %ipos "ipos"
               OpName %i "i"
               OpName %map "map"
               OpName %p "p"
               OpName %canwalk "canwalk"
               OpName %v "v"
               OpName %directions "directions"
               OpName %j "j"
               OpName %d "d"
               OpName %_GLF_color "_GLF_color"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpMemberDecorate %buf0 0 Offset 0
               OpDecorate %buf0 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %_GLF_color Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %buf0 = OpTypeStruct %v2float
%_ptr_Uniform_buf0 = OpTypePointer Uniform %buf0
          %_ = OpVariable %_ptr_Uniform_buf0 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
   %float_16 = OpConstant %float 16
     %uint_1 = OpConstant %uint 1
%_ptr_Function_int = OpTypePointer Function %int
    %int_256 = OpConstant %int 256
       %bool = OpTypeBool
   %uint_256 = OpConstant %uint 256
%_arr_int_uint_256 = OpTypeArray %int %uint_256
%_ptr_Private__arr_int_uint_256 = OpTypePointer Private %_arr_int_uint_256
        %map = OpVariable %_ptr_Private__arr_int_uint_256 Private
%_ptr_Private_int = OpTypePointer Private %int
      %int_1 = OpConstant %int 1
         %63 = OpConstantComposite %v2int %int_0 %int_0
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2
     %int_16 = OpConstant %int 16
     %int_14 = OpConstant %int 14
      %false = OpConstantFalse %bool
      %int_8 = OpConstant %int 8
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %_GLF_color = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
        %437 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %float_0 = OpConstant %float 0
        %441 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Private_mat2v4float = OpTypePointer Private %mat2v4float
        %556 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %557 = OpConstantComposite %mat2v4float %556 %556
        %558 = OpVariable %_ptr_Private_mat2v4float Private %557
        %760 = OpConstantNull %bool
       %main = OpFunction %void None %3
          %5 = OpLabel
        %pos = OpVariable %_ptr_Function_v2float Function
       %ipos = OpVariable %_ptr_Function_v2int Function
          %i = OpVariable %_ptr_Function_int Function
          %p = OpVariable %_ptr_Function_v2int Function
    %canwalk = OpVariable %_ptr_Function_bool Function
          %v = OpVariable %_ptr_Function_int Function
 %directions = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
          %d = OpVariable %_ptr_Function_int Function
         %13 = OpLoad %v4float %gl_FragCoord
         %14 = OpVectorShuffle %v2float %13 %13 0 1
        %564 = OpISub %int %int_256 %int_14
         %21 = OpAccessChain %_ptr_Uniform_v2float %_ %int_0
         %22 = OpLoad %v2float %21
        %566 = OpSNegate %int %564
         %23 = OpFDiv %v2float %14 %22
               OpStore %pos %23
         %30 = OpAccessChain %_ptr_Function_float %pos %uint_0
         %31 = OpLoad %float %30
         %33 = OpFMul %float %31 %float_16
         %34 = OpConvertFToS %int %33
         %36 = OpAccessChain %_ptr_Function_float %pos %uint_1
         %37 = OpLoad %float %36
         %38 = OpFMul %float %37 %float_16
         %39 = OpConvertFToS %int %38
         %40 = OpCompositeConstruct %v2int %34 %39
               OpStore %ipos %40
               OpStore %i %int_0
               OpBranch %43
         %43 = OpLabel
               OpLoopMerge %45 %46 None
               OpBranch %47
         %47 = OpLabel
         %48 = OpLoad %int %i
         %51 = OpSLessThan %bool %48 %int_256
               OpBranchConditional %51 %44 %45
         %44 = OpLabel
         %56 = OpLoad %int %i
         %58 = OpAccessChain %_ptr_Private_int %map %56
               OpStore %58 %int_0
               OpBranch %46
         %46 = OpLabel
         %59 = OpLoad %int %i
         %61 = OpIAdd %int %59 %int_1
               OpStore %i %61
               OpBranch %43
         %45 = OpLabel
               OpStore %p %63
               OpStore %canwalk %true
               OpStore %v %int_0
               OpBranch %68
         %68 = OpLabel
               OpLoopMerge %70 %71 None
               OpBranch %69
         %69 = OpLabel
         %72 = OpLoad %int %v
         %73 = OpIAdd %int %72 %int_1
               OpStore %v %73
               OpStore %directions %int_0
         %75 = OpAccessChain %_ptr_Function_int %p %uint_0
         %76 = OpLoad %int %75
         %77 = OpSGreaterThan %bool %76 %int_0
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
         %80 = OpAccessChain %_ptr_Function_int %p %uint_0
         %81 = OpLoad %int %80
         %83 = OpISub %int %81 %int_2
         %84 = OpAccessChain %_ptr_Function_int %p %uint_1
         %85 = OpLoad %int %84
         %87 = OpIMul %int %85 %int_16
         %88 = OpIAdd %int %83 %87
         %89 = OpAccessChain %_ptr_Private_int %map %88
         %90 = OpLoad %int %89
         %91 = OpIEqual %bool %90 %int_0
               OpBranch %79
         %79 = OpLabel
         %92 = OpPhi %bool %77 %69 %91 %78
               OpSelectionMerge %94 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
         %95 = OpLoad %int %directions
         %96 = OpIAdd %int %95 %int_1
               OpStore %directions %96
               OpBranch %94
         %94 = OpLabel
         %97 = OpAccessChain %_ptr_Function_int %p %uint_1
         %98 = OpLoad %int %97
         %99 = OpSGreaterThan %bool %98 %int_0
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
        %102 = OpAccessChain %_ptr_Function_int %p %uint_0
        %103 = OpLoad %int %102
        %104 = OpAccessChain %_ptr_Function_int %p %uint_1
        %105 = OpLoad %int %104
        %106 = OpISub %int %105 %int_2
        %107 = OpIMul %int %106 %int_16
        %108 = OpIAdd %int %103 %107
        %109 = OpAccessChain %_ptr_Private_int %map %108
        %110 = OpLoad %int %109
        %111 = OpIEqual %bool %110 %int_0
               OpBranch %101
        %101 = OpLabel
        %112 = OpPhi %bool %99 %94 %111 %100
               OpSelectionMerge %114 None
               OpBranchConditional %112 %113 %114
        %113 = OpLabel
        %115 = OpLoad %int %directions
        %116 = OpIAdd %int %115 %int_1
               OpStore %directions %116
               OpBranch %114
        %114 = OpLabel
        %117 = OpAccessChain %_ptr_Function_int %p %uint_0
        %118 = OpLoad %int %117
        %120 = OpSLessThan %bool %118 %int_14
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
        %123 = OpAccessChain %_ptr_Function_int %p %uint_0
        %124 = OpLoad %int %123
        %125 = OpIAdd %int %124 %int_2
        %126 = OpAccessChain %_ptr_Function_int %p %uint_1
        %127 = OpLoad %int %126
        %128 = OpIMul %int %127 %int_16
        %129 = OpIAdd %int %125 %128
        %130 = OpAccessChain %_ptr_Private_int %map %129
        %131 = OpLoad %int %130
        %132 = OpIEqual %bool %131 %int_0
               OpBranch %122
        %122 = OpLabel
        %133 = OpPhi %bool %120 %114 %132 %121
               OpSelectionMerge %135 None
               OpBranchConditional %133 %134 %135
        %134 = OpLabel
        %136 = OpLoad %int %directions
        %137 = OpIAdd %int %136 %int_1
               OpStore %directions %137
               OpBranch %135
        %135 = OpLabel
        %594 = OpISub %int %int_256 %566
        %138 = OpAccessChain %_ptr_Function_int %p %uint_1
        %139 = OpLoad %int %138
        %140 = OpSLessThan %bool %139 %int_14
               OpSelectionMerge %142 None
               OpBranchConditional %140 %141 %142
        %141 = OpLabel
        %143 = OpAccessChain %_ptr_Function_int %p %uint_0
        %144 = OpLoad %int %143
        %145 = OpAccessChain %_ptr_Function_int %p %uint_1
        %146 = OpLoad %int %145
        %147 = OpIAdd %int %146 %int_2
        %148 = OpIMul %int %147 %int_16
        %149 = OpIAdd %int %144 %148
        %150 = OpAccessChain %_ptr_Private_int %map %149
        %151 = OpLoad %int %150
        %152 = OpIEqual %bool %151 %int_0
               OpBranch %142
        %142 = OpLabel
        %153 = OpPhi %bool %140 %135 %152 %141
               OpSelectionMerge %155 None
               OpBranchConditional %153 %154 %155
        %154 = OpLabel
        %156 = OpLoad %int %directions
        %157 = OpIAdd %int %156 %int_1
               OpStore %directions %157
               OpBranch %155
        %155 = OpLabel
        %158 = OpLoad %int %directions
        %159 = OpIEqual %bool %158 %int_0
               OpSelectionMerge %161 None
               OpBranchConditional %159 %160 %207
        %160 = OpLabel
               OpStore %canwalk %false
               OpStore %i %int_0
               OpBranch %163
        %163 = OpLabel
               OpLoopMerge %165 %166 None
               OpBranch %167
        %167 = OpLabel
        %168 = OpLoad %int %i
        %170 = OpSLessThan %bool %168 %int_8
               OpBranchConditional %170 %164 %165
        %164 = OpLabel
               OpStore %j %int_0
        %609 = OpISub %int %594 %168
               OpStore %558 %557
               OpBranchConditional %760 %166 %172
        %172 = OpLabel
               OpLoopMerge %174 %175 Unroll
               OpBranch %176
        %176 = OpLabel
        %177 = OpLoad %int %j
        %178 = OpSLessThan %bool %177 %int_8
               OpBranchConditional %178 %173 %174
        %173 = OpLabel
        %179 = OpLoad %int %j
        %180 = OpIMul %int %179 %int_2
        %181 = OpLoad %int %i
        %182 = OpIMul %int %181 %int_2
        %183 = OpIMul %int %182 %int_16
        %184 = OpIAdd %int %180 %183
        %185 = OpAccessChain %_ptr_Private_int %map %184
        %186 = OpLoad %int %185
        %187 = OpIEqual %bool %186 %int_0
               OpSelectionMerge %189 None
               OpBranchConditional %187 %188 %189
        %188 = OpLabel
        %190 = OpLoad %int %j
        %191 = OpIMul %int %190 %int_2
        %192 = OpAccessChain %_ptr_Function_int %p %uint_0
               OpStore %192 %191
        %193 = OpLoad %int %i
        %194 = OpIMul %int %193 %int_2
        %195 = OpAccessChain %_ptr_Function_int %p %uint_1
               OpStore %195 %194
               OpStore %canwalk %true
               OpBranch %189
        %189 = OpLabel
               OpBranch %175
        %175 = OpLabel
        %196 = OpLoad %int %j
        %197 = OpIAdd %int %196 %int_1
               OpStore %j %197
               OpBranch %172
        %174 = OpLabel
               OpBranch %166
        %166 = OpLabel
        %198 = OpLoad %int %i
        %199 = OpIAdd %int %198 %int_1
               OpStore %i %199
               OpBranch %163
        %165 = OpLabel
        %200 = OpAccessChain %_ptr_Function_int %p %uint_0
        %201 = OpLoad %int %200
        %202 = OpAccessChain %_ptr_Function_int %p %uint_1
        %203 = OpLoad %int %202
        %204 = OpIMul %int %203 %int_16
        %205 = OpIAdd %int %201 %204
        %206 = OpAccessChain %_ptr_Private_int %map %205
               OpStore %206 %int_1
               OpBranch %161
        %207 = OpLabel
        %209 = OpLoad %int %v
        %210 = OpLoad %int %directions
        %211 = OpSMod %int %209 %210
               OpStore %d %211
        %212 = OpLoad %int %directions
        %213 = OpLoad %int %v
        %214 = OpIAdd %int %213 %212
               OpStore %v %214
        %215 = OpLoad %int %d
        %216 = OpSGreaterThanEqual %bool %215 %int_0
               OpSelectionMerge %218 None
               OpBranchConditional %216 %217 %218
        %217 = OpLabel
        %219 = OpAccessChain %_ptr_Function_int %p %uint_0
        %220 = OpLoad %int %219
        %221 = OpSGreaterThan %bool %220 %int_0
               OpBranch %218
        %218 = OpLabel
        %222 = OpPhi %bool %216 %207 %221 %217
               OpSelectionMerge %224 None
               OpBranchConditional %222 %223 %224
        %223 = OpLabel
        %225 = OpAccessChain %_ptr_Function_int %p %uint_0
        %226 = OpLoad %int %225
        %227 = OpISub %int %226 %int_2
        %228 = OpAccessChain %_ptr_Function_int %p %uint_1
        %229 = OpLoad %int %228
        %230 = OpIMul %int %229 %int_16
        %231 = OpIAdd %int %227 %230
        %232 = OpAccessChain %_ptr_Private_int %map %231
        %233 = OpLoad %int %232
        %234 = OpIEqual %bool %233 %int_0
               OpBranch %224
        %224 = OpLabel
        %235 = OpPhi %bool %222 %218 %234 %223
               OpSelectionMerge %237 None
               OpBranchConditional %235 %236 %237
        %236 = OpLabel
        %238 = OpLoad %int %d
        %239 = OpISub %int %238 %int_1
               OpStore %d %239
        %240 = OpAccessChain %_ptr_Function_int %p %uint_0
        %241 = OpLoad %int %240
        %242 = OpAccessChain %_ptr_Function_int %p %uint_1
        %243 = OpLoad %int %242
        %244 = OpIMul %int %243 %int_16
        %245 = OpIAdd %int %241 %244
        %246 = OpAccessChain %_ptr_Private_int %map %245
               OpStore %246 %int_1
        %247 = OpAccessChain %_ptr_Function_int %p %uint_0
        %248 = OpLoad %int %247
        %249 = OpISub %int %248 %int_1
        %250 = OpAccessChain %_ptr_Function_int %p %uint_1
        %251 = OpLoad %int %250
        %252 = OpIMul %int %251 %int_16
        %253 = OpIAdd %int %249 %252
        %254 = OpAccessChain %_ptr_Private_int %map %253
               OpStore %254 %int_1
        %255 = OpAccessChain %_ptr_Function_int %p %uint_0
        %256 = OpLoad %int %255
        %257 = OpISub %int %256 %int_2
        %258 = OpAccessChain %_ptr_Function_int %p %uint_1
        %259 = OpLoad %int %258
        %260 = OpIMul %int %259 %int_16
        %261 = OpIAdd %int %257 %260
        %262 = OpAccessChain %_ptr_Private_int %map %261
               OpStore %262 %int_1
        %263 = OpAccessChain %_ptr_Function_int %p %uint_0
        %264 = OpLoad %int %263
        %265 = OpISub %int %264 %int_2
        %266 = OpAccessChain %_ptr_Function_int %p %uint_0
               OpStore %266 %265
               OpBranch %237
        %237 = OpLabel
        %267 = OpLoad %int %d
        %268 = OpSGreaterThanEqual %bool %267 %int_0
               OpSelectionMerge %270 None
               OpBranchConditional %268 %269 %270
        %269 = OpLabel
        %271 = OpAccessChain %_ptr_Function_int %p %uint_1
        %272 = OpLoad %int %271
        %273 = OpSGreaterThan %bool %272 %int_0
               OpBranch %270
        %270 = OpLabel
        %274 = OpPhi %bool %268 %237 %273 %269
               OpSelectionMerge %276 None
               OpBranchConditional %274 %275 %276
        %275 = OpLabel
        %277 = OpAccessChain %_ptr_Function_int %p %uint_0
        %278 = OpLoad %int %277
        %279 = OpAccessChain %_ptr_Function_int %p %uint_1
        %280 = OpLoad %int %279
        %281 = OpISub %int %280 %int_2
        %282 = OpIMul %int %281 %int_16
        %283 = OpIAdd %int %278 %282
        %284 = OpAccessChain %_ptr_Private_int %map %283
        %285 = OpLoad %int %284
        %286 = OpIEqual %bool %285 %int_0
               OpBranch %276
        %276 = OpLabel
        %287 = OpPhi %bool %274 %270 %286 %275
               OpSelectionMerge %289 None
               OpBranchConditional %287 %288 %289
        %288 = OpLabel
        %290 = OpLoad %int %d
        %291 = OpISub %int %290 %int_1
               OpStore %d %291
        %292 = OpAccessChain %_ptr_Function_int %p %uint_0
        %293 = OpLoad %int %292
        %294 = OpAccessChain %_ptr_Function_int %p %uint_1
        %295 = OpLoad %int %294
        %296 = OpIMul %int %295 %int_16
        %297 = OpIAdd %int %293 %296
        %298 = OpAccessChain %_ptr_Private_int %map %297
               OpStore %298 %int_1
        %299 = OpAccessChain %_ptr_Function_int %p %uint_0
        %300 = OpLoad %int %299
        %301 = OpAccessChain %_ptr_Function_int %p %uint_1
        %302 = OpLoad %int %301
        %303 = OpISub %int %302 %int_1
        %304 = OpIMul %int %303 %int_16
        %305 = OpIAdd %int %300 %304
        %306 = OpAccessChain %_ptr_Private_int %map %305
               OpStore %306 %int_1
        %307 = OpAccessChain %_ptr_Function_int %p %uint_0
        %308 = OpLoad %int %307
        %309 = OpAccessChain %_ptr_Function_int %p %uint_1
        %310 = OpLoad %int %309
        %311 = OpISub %int %310 %int_2
        %312 = OpIMul %int %311 %int_16
        %313 = OpIAdd %int %308 %312
        %314 = OpAccessChain %_ptr_Private_int %map %313
               OpStore %314 %int_1
        %315 = OpAccessChain %_ptr_Function_int %p %uint_1
        %316 = OpLoad %int %315
        %317 = OpISub %int %316 %int_2
        %318 = OpAccessChain %_ptr_Function_int %p %uint_1
               OpStore %318 %317
               OpBranch %289
        %289 = OpLabel
        %319 = OpLoad %int %d
        %320 = OpSGreaterThanEqual %bool %319 %int_0
               OpSelectionMerge %322 None
               OpBranchConditional %320 %321 %322
        %321 = OpLabel
        %323 = OpAccessChain %_ptr_Function_int %p %uint_0
        %324 = OpLoad %int %323
        %325 = OpSLessThan %bool %324 %int_14
               OpBranch %322
        %322 = OpLabel
        %326 = OpPhi %bool %320 %289 %325 %321
               OpSelectionMerge %328 None
               OpBranchConditional %326 %327 %328
        %327 = OpLabel
        %329 = OpAccessChain %_ptr_Function_int %p %uint_0
        %330 = OpLoad %int %329
        %331 = OpIAdd %int %330 %int_2
        %332 = OpAccessChain %_ptr_Function_int %p %uint_1
        %333 = OpLoad %int %332
        %334 = OpIMul %int %333 %int_16
        %335 = OpIAdd %int %331 %334
        %336 = OpAccessChain %_ptr_Private_int %map %335
        %337 = OpLoad %int %336
        %338 = OpIEqual %bool %337 %int_0
               OpBranch %328
        %328 = OpLabel
        %339 = OpPhi %bool %326 %322 %338 %327
               OpSelectionMerge %341 None
               OpBranchConditional %339 %340 %341
        %340 = OpLabel
        %342 = OpLoad %int %d
        %343 = OpISub %int %342 %int_1
               OpStore %d %343
        %344 = OpAccessChain %_ptr_Function_int %p %uint_0
        %345 = OpLoad %int %344
        %346 = OpAccessChain %_ptr_Function_int %p %uint_1
        %347 = OpLoad %int %346
        %348 = OpIMul %int %347 %int_16
        %349 = OpIAdd %int %345 %348
        %350 = OpAccessChain %_ptr_Private_int %map %349
               OpStore %350 %int_1
        %351 = OpAccessChain %_ptr_Function_int %p %uint_0
        %352 = OpLoad %int %351
        %353 = OpIAdd %int %352 %int_1
        %354 = OpAccessChain %_ptr_Function_int %p %uint_1
        %355 = OpLoad %int %354
        %356 = OpIMul %int %355 %int_16
        %357 = OpIAdd %int %353 %356
        %358 = OpAccessChain %_ptr_Private_int %map %357
               OpStore %358 %int_1
        %359 = OpAccessChain %_ptr_Function_int %p %uint_0
        %360 = OpLoad %int %359
        %361 = OpIAdd %int %360 %int_2
        %362 = OpAccessChain %_ptr_Function_int %p %uint_1
        %363 = OpLoad %int %362
        %364 = OpIMul %int %363 %int_16
        %365 = OpIAdd %int %361 %364
        %366 = OpAccessChain %_ptr_Private_int %map %365
               OpStore %366 %int_1
        %367 = OpAccessChain %_ptr_Function_int %p %uint_0
        %368 = OpLoad %int %367
        %369 = OpIAdd %int %368 %int_2
        %370 = OpAccessChain %_ptr_Function_int %p %uint_0
               OpStore %370 %369
               OpBranch %341
        %341 = OpLabel
        %371 = OpLoad %int %d
        %372 = OpSGreaterThanEqual %bool %371 %int_0
               OpSelectionMerge %374 None
               OpBranchConditional %372 %373 %374
        %373 = OpLabel
        %375 = OpAccessChain %_ptr_Function_int %p %uint_1
        %376 = OpLoad %int %375
        %377 = OpSLessThan %bool %376 %int_14
               OpBranch %374
        %374 = OpLabel
        %378 = OpPhi %bool %372 %341 %377 %373
               OpSelectionMerge %380 None
               OpBranchConditional %378 %379 %380
        %379 = OpLabel
        %381 = OpAccessChain %_ptr_Function_int %p %uint_0
        %382 = OpLoad %int %381
        %383 = OpAccessChain %_ptr_Function_int %p %uint_1
        %384 = OpLoad %int %383
        %385 = OpIAdd %int %384 %int_2
        %386 = OpIMul %int %385 %int_16
        %387 = OpIAdd %int %382 %386
        %388 = OpAccessChain %_ptr_Private_int %map %387
        %389 = OpLoad %int %388
        %390 = OpIEqual %bool %389 %int_0
               OpBranch %380
        %380 = OpLabel
        %391 = OpPhi %bool %378 %374 %390 %379
               OpSelectionMerge %393 None
               OpBranchConditional %391 %392 %393
        %392 = OpLabel
        %394 = OpLoad %int %d
        %395 = OpISub %int %394 %int_1
               OpStore %d %395
        %396 = OpAccessChain %_ptr_Function_int %p %uint_0
        %397 = OpLoad %int %396
        %398 = OpAccessChain %_ptr_Function_int %p %uint_1
        %399 = OpLoad %int %398
        %400 = OpIMul %int %399 %int_16
        %401 = OpIAdd %int %397 %400
        %402 = OpAccessChain %_ptr_Private_int %map %401
               OpStore %402 %int_1
        %403 = OpAccessChain %_ptr_Function_int %p %uint_0
        %404 = OpLoad %int %403
        %405 = OpAccessChain %_ptr_Function_int %p %uint_1
        %406 = OpLoad %int %405
        %407 = OpIAdd %int %406 %int_1
        %408 = OpIMul %int %407 %int_16
        %409 = OpIAdd %int %404 %408
        %410 = OpAccessChain %_ptr_Private_int %map %409
               OpStore %410 %int_1
        %411 = OpAccessChain %_ptr_Function_int %p %uint_0
        %412 = OpLoad %int %411
        %413 = OpAccessChain %_ptr_Function_int %p %uint_1
        %414 = OpLoad %int %413
        %415 = OpIAdd %int %414 %int_2
        %416 = OpIMul %int %415 %int_16
        %417 = OpIAdd %int %412 %416
        %418 = OpAccessChain %_ptr_Private_int %map %417
               OpStore %418 %int_1
        %419 = OpAccessChain %_ptr_Function_int %p %uint_1
        %420 = OpLoad %int %419
        %421 = OpIAdd %int %420 %int_2
        %422 = OpAccessChain %_ptr_Function_int %p %uint_1
               OpStore %422 %421
               OpBranch %393
        %393 = OpLabel
               OpBranch %161
        %161 = OpLabel
        %423 = OpAccessChain %_ptr_Function_int %ipos %uint_1
        %424 = OpLoad %int %423
        %425 = OpIMul %int %424 %int_16
        %426 = OpAccessChain %_ptr_Function_int %ipos %uint_0
        %427 = OpLoad %int %426
        %428 = OpIAdd %int %425 %427
        %429 = OpAccessChain %_ptr_Private_int %map %428
        %430 = OpLoad %int %429
        %431 = OpIEqual %bool %430 %int_1
               OpSelectionMerge %433 None
               OpBranchConditional %431 %432 %433
        %432 = OpLabel
               OpStore %_GLF_color %437
               OpReturn
        %433 = OpLabel
               OpBranch %71
         %71 = OpLabel
        %439 = OpLoad %bool %canwalk
               OpBranchConditional %439 %68 %70
         %70 = OpLabel
               OpStore %_GLF_color %441
               OpReturn
               OpFunctionEnd
