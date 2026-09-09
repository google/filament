; RUN: %dxopt %s -scalarizer -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWStructuredBuffer<vector<float, 1> >" = type { <1 x float> }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }

@"\01?buf@@3V?$RWStructuredBuffer@V?$vector@M$00@@@@A" = external global %"class.RWStructuredBuffer<vector<float, 1> >", align 4
@llvm.used = appending global [1 x i8*] [i8* bitcast (%"class.RWStructuredBuffer<vector<float, 1> >"* @"\01?buf@@3V?$RWStructuredBuffer@V?$vector@M$00@@@@A" to i8*)], section "llvm.metadata"

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?assignments
define void @"\01?assignments@@YAXY09$$CAV?$vector@M$00@@@Z"([10 x <1 x float>]* noalias %things) #0 {
bb:
  %tmp = load %"class.RWStructuredBuffer<vector<float, 1> >", %"class.RWStructuredBuffer<vector<float, 1> >"* @"\01?buf@@3V?$RWStructuredBuffer@V?$vector@M$00@@@@A"
  %tmp1 = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<vector<float, 1> >"(i32 160, %"class.RWStructuredBuffer<vector<float, 1> >" %tmp)
  %tmp2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4108, i32 4 })
  %RawBufferLoad = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp2, i32 1, i32 0, i8 1, i32 4)
  %tmp3 = extractvalue %dx.types.ResRet.f32 %RawBufferLoad, 0
  %tmp4 = insertelement <1 x float> undef, float %tmp3, i64 0
  %tmp5 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 0
  store <1 x float> %tmp4, <1 x float>* %tmp5, align 4

  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <1 x float>, <1 x float>* [[adr5]]
  ; CHECK: [[val5:%.*]] = extractelement <1 x float> [[ld5]], i32 0
  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <1 x float>, <1 x float>* [[adr1]]
  ; CHECK: [[val1:%.*]] = extractelement <1 x float> [[ld1]], i32 0
  ; CHECK: [[res1:%.*]] = fadd fast float [[val1]], [[val5]]
  ; CHECK: [[vec1:%.*]] = insertelement <1 x float> undef, float [[res1]], i32 0
  ; CHECK: store <1 x float> [[vec1]], <1 x float>* [[adr1]], align 4
  %tmp6 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 5
  %tmp7 = load <1 x float>, <1 x float>* %tmp6, align 4
  %tmp8 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 1
  %tmp9 = load <1 x float>, <1 x float>* %tmp8, align 4
  %tmp10 = fadd fast <1 x float> %tmp9, %tmp7
  store <1 x float> %tmp10, <1 x float>* %tmp8, align 4

  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load <1 x float>, <1 x float>* [[adr6]]
  ; CHECK: [[val6:%.*]] = extractelement <1 x float> [[ld6]], i32 0
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <1 x float>, <1 x float>* [[adr2]]
  ; CHECK: [[val2:%.*]] = extractelement <1 x float> [[ld2]], i32 0
  ; CHECK: [[res2:%.*]] = fsub fast float [[val2]], [[val6]]
  ; CHECK: [[vec2:%.*]] = insertelement <1 x float> undef, float [[res2]], i32 0
  ; CHECK: store <1 x float> [[vec2]], <1 x float>* [[adr2]], align 4
  %tmp11 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 6
  %tmp12 = load <1 x float>, <1 x float>* %tmp11, align 4
  %tmp13 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 2
  %tmp14 = load <1 x float>, <1 x float>* %tmp13, align 4
  %tmp15 = fsub fast <1 x float> %tmp14, %tmp12
  store <1 x float> %tmp15, <1 x float>* %tmp13, align 4

  ; CHECK: [[adr7:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 7
  ; CHECK: [[ld7:%.*]] = load <1 x float>, <1 x float>* [[adr7]]
  ; CHECK: [[val7:%.*]] = extractelement <1 x float> [[ld7]], i32 0
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <1 x float>, <1 x float>* [[adr3]]
  ; CHECK: [[val3:%.*]] = extractelement <1 x float> [[ld3]], i32 0
  ; CHECK: [[res3:%.*]] = fmul fast float [[val3]], [[val7]]
  ; CHECK: [[vec3:%.*]] = insertelement <1 x float> undef, float [[res3]], i32 0
  ; CHECK: store <1 x float> [[vec3]], <1 x float>* [[adr3]], align 4
  %tmp16 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 7
  %tmp17 = load <1 x float>, <1 x float>* %tmp16, align 4
  %tmp18 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 3
  %tmp19 = load <1 x float>, <1 x float>* %tmp18, align 4
  %tmp20 = fmul fast <1 x float> %tmp19, %tmp17
  store <1 x float> %tmp20, <1 x float>* %tmp18, align 4

  ; CHECK: [[adr8:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 8
  ; CHECK: [[ld8:%.*]] = load <1 x float>, <1 x float>* [[adr8]]
  ; CHECK: [[val8:%.*]] = extractelement <1 x float> [[ld8]], i32 0
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <1 x float>, <1 x float>* [[adr4]]
  ; CHECK: [[val4:%.*]] = extractelement <1 x float> [[ld4]], i32 0
  ; CHECK: [[res4:%.*]] = fdiv fast float [[val4]], [[val8]]
  ; CHECK: [[vec4:%.*]] = insertelement <1 x float> undef, float [[res4]], i32 0
  ; CHECK: store <1 x float> [[vec4]], <1 x float>* [[adr4]], align 4
  %tmp21 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 8
  %tmp22 = load <1 x float>, <1 x float>* %tmp21, align 4
  %tmp23 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 4
  %tmp24 = load <1 x float>, <1 x float>* %tmp23, align 4
  %tmp25 = fdiv fast <1 x float> %tmp24, %tmp22
  store <1 x float> %tmp25, <1 x float>* %tmp23, align 4

  ; CHECK: [[adr9:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 9
  ; CHECK: [[ld9:%.*]] = load <1 x float>, <1 x float>* [[adr9]]
  ; CHECK: [[val9:%.*]] = extractelement <1 x float> [[ld9]], i32 0
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <1 x float>, <1 x float>* [[adr5]]
  ; CHECK: [[val5:%.*]] = extractelement <1 x float> [[ld5]], i32 0
  ; CHECK: [[res5:%.*]] = frem fast float [[val5]], [[val9]]
  ; CHECK: [[vec5:%.*]] = insertelement <1 x float> undef, float [[res5]], i32 0
  ; CHECK: store <1 x float> [[vec5]], <1 x float>* [[adr5]], align 4
  %tmp26 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 9
  %tmp27 = load <1 x float>, <1 x float>* %tmp26, align 4
  %tmp28 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 5
  %tmp29 = load <1 x float>, <1 x float>* %tmp28, align 4
  %tmp30 = frem fast <1 x float> %tmp29, %tmp27
  store <1 x float> %tmp30, <1 x float>* %tmp28, align 4

  ret void
}

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?arithmetic
define void @"\01?arithmetic@@YA$$BY0L@V?$vector@M$00@@Y0L@$$CAV1@@Z"([11 x <1 x float>]* noalias sret %agg.result, [11 x <1 x float>]* noalias %things) #0 {
bb:
  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 0
  ; CHECK: [[ld0:%.*]] = load <1 x float>, <1 x float>* [[adr0]], align 4
  ; CHECK-DAG: [[zero:%.*]] = extractelement <1 x float> <float -0.000000e+00>, i32 0
  ; CHECK-DAG: [[val0:%.*]] = extractelement <1 x float> [[ld0]], i32 0
  ; CHECK: [[sub0:%.*]] = fsub fast float [[zero]], [[val0]]
  ; CHECK: [[res0:%.*]] = insertelement <1 x float> undef, float [[sub0]], i32 0
  %tmp = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 0
  %tmp1 = load <1 x float>, <1 x float>* %tmp, align 4
  %tmp2 = fsub fast <1 x float> <float -0.000000e+00>, %tmp1
  %tmp3 = extractelement <1 x float> %tmp2, i64 0

  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 0
  ; CHECK: [[res1:%.*]] = load <1 x float>, <1 x float>* [[adr0]], align 4
  ; CHECK: [[val0:%.*]] = extractelement <1 x float> [[res1]], i64 0
  %tmp4 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 0
  %tmp5 = load <1 x float>, <1 x float>* %tmp4, align 4
  %tmp6 = extractelement <1 x float> %tmp5, i64 0

  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <1 x float>, <1 x float>* [[adr1]], align 4
  ; CHECK: [[val1:%.*]] = extractelement <1 x float> [[ld1]], i32 0
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <1 x float>, <1 x float>* [[adr2]], align 4
  ; CHECK: [[val2:%.*]] = extractelement <1 x float> [[ld2]], i32 0
  ; CHECK: [[add1:%.*]] = fadd fast float [[val1]], [[val2]]
  ; CHECK: [[res1:%.*]] = insertelement <1 x float> undef, float [[add1]], i32 0
  %tmp7 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 1
  %tmp8 = load <1 x float>, <1 x float>* %tmp7, align 4
  %tmp9 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 2
  %tmp10 = load <1 x float>, <1 x float>* %tmp9, align 4
  %tmp11 = fadd fast <1 x float> %tmp8, %tmp10
  %tmp12 = extractelement <1 x float> %tmp11, i64 0

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <1 x float>, <1 x float>* [[adr2]], align 4
  ; CHECK: [[val2:%.*]] = extractelement <1 x float> [[ld2]], i32 0
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <1 x float>, <1 x float>* [[adr3]], align 4
  ; CHECK: [[val3:%.*]] = extractelement <1 x float> [[ld3]], i32 0
  ; CHECK: [[sub2:%.*]] = fsub fast float [[val2]], [[val3]]
  ; CHECK: [[res2:%.*]] = insertelement <1 x float> undef, float [[sub2]], i32 0
  %tmp13 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 2
  %tmp14 = load <1 x float>, <1 x float>* %tmp13, align 4
  %tmp15 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 3
  %tmp16 = load <1 x float>, <1 x float>* %tmp15, align 4
  %tmp17 = fsub fast <1 x float> %tmp14, %tmp16
  %tmp18 = extractelement <1 x float> %tmp17, i64 0

  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <1 x float>, <1 x float>* [[adr3]], align 4
  ; CHECK: [[val3:%.*]] = extractelement <1 x float> [[ld3]], i32 0
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <1 x float>, <1 x float>* [[adr4]], align 4
  ; CHECK: [[val4:%.*]] = extractelement <1 x float> [[ld4]], i32 0
  ; CHECK: [[mul3:%.*]] = fmul fast float [[val3]], [[val4]]
  ; CHECK: [[res3:%.*]] = insertelement <1 x float> undef, float [[mul3]], i32 0
  %tmp19 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 3
  %tmp20 = load <1 x float>, <1 x float>* %tmp19, align 4
  %tmp21 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 4
  %tmp22 = load <1 x float>, <1 x float>* %tmp21, align 4
  %tmp23 = fmul fast <1 x float> %tmp20, %tmp22
  %tmp24 = extractelement <1 x float> %tmp23, i64 0

  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <1 x float>, <1 x float>* [[adr4]], align 4
  ; CHECK: [[val4:%.*]] = extractelement <1 x float> [[ld4]], i32 0
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <1 x float>, <1 x float>* [[adr5]], align 4
  ; CHECK: [[val5:%.*]] = extractelement <1 x float> [[ld5]], i32 0
  ; CHECK: [[div4:%.*]] = fdiv fast float [[val4]], [[val5]]
  ; CHECK: [[res4:%.*]] = insertelement <1 x float> undef, float [[div4]], i32 0
  %tmp25 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 4
  %tmp26 = load <1 x float>, <1 x float>* %tmp25, align 4
  %tmp27 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 5
  %tmp28 = load <1 x float>, <1 x float>* %tmp27, align 4
  %tmp29 = fdiv fast <1 x float> %tmp26, %tmp28
  %tmp30 = extractelement <1 x float> %tmp29, i64 0

  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <1 x float>, <1 x float>* [[adr5]], align 4
  ; CHECK: [[val5:%.*]] = extractelement <1 x float> [[ld5]], i32 0
  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load <1 x float>, <1 x float>* [[adr6]], align 4
  ; CHECK: [[val6:%.*]] = extractelement <1 x float> [[ld6]], i32 0
  ; CHECK: [[rem5:%.*]] = frem fast float [[val5]], [[val6]]
  ; CHECK: [[res5:%.*]] = insertelement <1 x float> undef, float [[rem5]], i32 0
  %tmp31 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 5
  %tmp32 = load <1 x float>, <1 x float>* %tmp31, align 4
  %tmp33 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 6
  %tmp34 = load <1 x float>, <1 x float>* %tmp33, align 4
  %tmp35 = frem fast <1 x float> %tmp32, %tmp34
  %tmp36 = extractelement <1 x float> %tmp35, i64 0

  ; CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 7
  ; CHECK: [[ld7:%.*]] = load <1 x float>, <1 x float>* [[adr7]], align 4
  ; CHECK-DAG: [[val7:%.*]] = extractelement <1 x float> [[ld7]], i32 0
  ; CHECK-DAG: [[pos1:%.*]] = extractelement <1 x float> <float 1.000000e+00>, i32 0
  ; CHECK: [[add6:%.*]] = fadd fast float [[val7]], [[pos1]]
  ; CHECK: [[res6:%.*]] = insertelement <1 x float> undef, float [[add6]], i32 0
  %tmp37 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 7
  %tmp38 = load <1 x float>, <1 x float>* %tmp37, align 4
  %tmp39 = fadd fast <1 x float> %tmp38, <float 1.000000e+00>
  store <1 x float> %tmp39, <1 x float>* %tmp37, align 4

  ; CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 8
  ; CHECK: [[ld8:%.*]] = load <1 x float>, <1 x float>* [[adr8]], align 4
  ; CHECK-DAG: [[val8:%.*]] = extractelement <1 x float> [[ld8]], i32 0
  ; CHECK-DAG: [[neg1:%.*]] = extractelement <1 x float> <float -1.000000e+00>, i32 0
  ; CHECK: [[add7:%.*]] = fadd fast float [[val8]], [[neg1]]
  ; CHECK: [[res7:%.*]] = insertelement <1 x float> undef, float [[add7]], i32 0
  %tmp40 = extractelement <1 x float> %tmp38, i64 0
  %tmp41 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 8
  %tmp42 = load <1 x float>, <1 x float>* %tmp41, align 4
  %tmp43 = fadd fast <1 x float> %tmp42, <float -1.000000e+00>
  store <1 x float> %tmp43, <1 x float>* %tmp41, align 4

  ; CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 9
  ; CHECK: [[ld9:%.*]] = load <1 x float>, <1 x float>* [[adr9]], align 4
  ; CHECK-DAG: [[val9:%.*]] = extractelement <1 x float> [[ld9]], i32 0
  ; CHECK-DAG: [[pos1:%.*]] = extractelement <1 x float> <float 1.000000e+00>, i32 0
  ; CHECK: [[add8:%.*]] = fadd fast float [[val9]], [[pos1]]
  ; CHECK: [[res8:%.*]] = insertelement <1 x float> undef, float [[add8]], i32 0
  %tmp44 = extractelement <1 x float> %tmp42, i64 0
  %tmp45 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 9
  %tmp46 = load <1 x float>, <1 x float>* %tmp45, align 4
  %tmp47 = fadd fast <1 x float> %tmp46, <float 1.000000e+00>
  store <1 x float> %tmp47, <1 x float>* %tmp45, align 4

  ; CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 10
  ; CHECK: [[ld10:%.*]] = load <1 x float>, <1 x float>* [[adr10]], align 4
  ; CHECK-DAG: [[val10:%.*]] = extractelement <1 x float> [[ld10]], i32 0
  ; CHECK-DAG: [[neg1:%.*]] = extractelement <1 x float> <float -1.000000e+00>, i32 0
  ; CHECK: [[add9:%.*]] = fadd fast float [[val10]], [[neg1]]
  ; CHECK: [[res9:%.*]] = insertelement <1 x float> undef, float [[add9]], i32 0
  %tmp48 = extractelement <1 x float> %tmp47, i64 0
  %tmp49 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %things, i32 0, i32 10
  %tmp50 = load <1 x float>, <1 x float>* %tmp49, align 4
  %tmp51 = fadd fast <1 x float> %tmp50, <float -1.000000e+00>
  store <1 x float> %tmp51, <1 x float>* %tmp49, align 4

  %tmp52 = extractelement <1 x float> %tmp51, i64 0
  %tmp53 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 0
  %insert20 = insertelement <1 x float> undef, float %tmp3, i64 0
  store <1 x float> %insert20, <1 x float>* %tmp53
  %tmp54 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 1
  %insert18 = insertelement <1 x float> undef, float %tmp6, i64 0
  store <1 x float> %insert18, <1 x float>* %tmp54
  %tmp55 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 2
  %insert16 = insertelement <1 x float> undef, float %tmp12, i64 0
  store <1 x float> %insert16, <1 x float>* %tmp55
  %tmp56 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 3
  %insert14 = insertelement <1 x float> undef, float %tmp18, i64 0
  store <1 x float> %insert14, <1 x float>* %tmp56
  %tmp57 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 4
  %insert12 = insertelement <1 x float> undef, float %tmp24, i64 0
  store <1 x float> %insert12, <1 x float>* %tmp57
  %tmp58 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 5
  %insert10 = insertelement <1 x float> undef, float %tmp30, i64 0
  store <1 x float> %insert10, <1 x float>* %tmp58
  %tmp59 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 6
  %insert8 = insertelement <1 x float> undef, float %tmp36, i64 0
  store <1 x float> %insert8, <1 x float>* %tmp59
  %tmp60 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 7
  %insert6 = insertelement <1 x float> undef, float %tmp40, i64 0
  store <1 x float> %insert6, <1 x float>* %tmp60
  %tmp61 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 8
  %insert4 = insertelement <1 x float> undef, float %tmp44, i64 0
  store <1 x float> %insert4, <1 x float>* %tmp61
  %tmp62 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 9
  %insert2 = insertelement <1 x float> undef, float %tmp48, i64 0
  store <1 x float> %insert2, <1 x float>* %tmp62
  %tmp63 = getelementptr inbounds [11 x <1 x float>], [11 x <1 x float>]* %agg.result, i32 0, i32 10
  %insert = insertelement <1 x float> undef, float %tmp52, i64 0
  store <1 x float> %insert, <1 x float>* %tmp63
  ret void
}

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?logic
define void @"\01?logic@@YA$$BY09_NY09_NY09V?$vector@M$00@@@Z"([10 x i32]* noalias sret %agg.result, [10 x i32]* %truth, [10 x <1 x float>]* %consequences) #0 {
bb:
  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 0
  ; CHECK: [[ld0:%.*]] = load i32, i32* [[adr0]], align 4
  ; CHECK: [[cmp0:%.*]] = icmp ne i32 [[ld0]], 0
  ; CHECK: [[bres0:%.*]] = xor i1 [[cmp0]], true
  ; CHECK: [[res0:%.*]] = zext i1 [[bres0]] to i32
  %tmp = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 0
  %tmp1 = load i32, i32* %tmp, align 4
  %tmp2 = icmp ne i32 %tmp1, 0
  %tmp3 = xor i1 %tmp2, true
  %tmp4 = zext i1 %tmp3 to i32

  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load i32, i32* [[adr1]], align 4
  ; CHECK: [[cmp1:%.*]] = icmp ne i32 [[ld1]], 0
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load i32, i32* [[adr2]], align 4
  ; CHECK: [[cmp2:%.*]] = icmp ne i32 [[ld2]], 0
  ; CHECK: [[bres1:%.*]] = or i1 [[cmp1]], [[cmp2]]
  ; CHECK: [[res1:%.*]] = zext i1 [[bres1]] to i32
  %tmp5 = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 1
  %tmp6 = load i32, i32* %tmp5, align 4
  %tmp7 = icmp ne i32 %tmp6, 0
  %tmp9 = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 2
  %tmp10 = load i32, i32* %tmp9, align 4
  %tmp11 = icmp ne i32 %tmp10, 0
  %tmp13 = or i1 %tmp7, %tmp11
  %tmp14 = zext i1 %tmp13 to i32

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load i32, i32* [[adr2]], align 4
  ; CHECK: [[cmp2:%.*]] = icmp ne i32 [[ld2]], 0
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load i32, i32* [[adr3]], align 4
  ; CHECK: [[cmp3:%.*]] = icmp ne i32 [[ld3]], 0
  ; CHECK: [[bres2:%.*]] = and i1 [[cmp2]], [[cmp3]]
  ; CHECK: [[res2:%.*]] = zext i1 [[bres2]] to i32
  %tmp15 = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 2
  %tmp16 = load i32, i32* %tmp15, align 4
  %tmp17 = icmp ne i32 %tmp16, 0
  %tmp19 = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 3
  %tmp20 = load i32, i32* %tmp19, align 4
  %tmp21 = icmp ne i32 %tmp20, 0
  %tmp23 = and i1 %tmp17, %tmp21
  %tmp24 = zext i1 %tmp23 to i32

  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load i32, i32* [[adr3]], align 4
  ; CHECK: [[cmp3:%.*]] = icmp ne i32 [[ld3]], 0
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load i32, i32* [[adr4]], align 4
  ; CHECK: [[cmp4:%.*]] = icmp ne i32 [[ld4]], 0
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load i32, i32* [[adr5]], align 4
  ; CHECK: [[cmp5:%.*]] = icmp ne i32 [[ld5]], 0
  ; CHECK: [[bres3:%.*]] = select i1 [[cmp3]], i1 [[cmp4]], i1 [[cmp5]]
  ; CHECK: [[res3:%.*]] = zext i1 [[bres3]] to i32
  %tmp25 = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 3
  %tmp26 = load i32, i32* %tmp25, align 4
  %tmp27 = icmp ne i32 %tmp26, 0
  %tmp29 = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 4
  %tmp30 = load i32, i32* %tmp29, align 4
  %tmp31 = icmp ne i32 %tmp30, 0
  %tmp32 = getelementptr inbounds [10 x i32], [10 x i32]* %truth, i32 0, i32 5
  %tmp33 = load i32, i32* %tmp32, align 4
  %tmp34 = icmp ne i32 %tmp33, 0
  %tmp35 = select i1 %tmp27, i1 %tmp31, i1 %tmp34
  %tmp36 = zext i1 %tmp35 to i32

  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 0
  ; CHECK: [[ld0:%.*]] = load <1 x float>, <1 x float>* [[adr0]]
  ; CHECK: [[val0:%.*]] = extractelement <1 x float> [[ld0]], i32 0
  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <1 x float>, <1 x float>* [[adr1]]
  ; CHECK: [[val1:%.*]] = extractelement <1 x float> [[ld1]], i32 0
  ; CHECK: [[bres4:%.*]] = fcmp fast oeq float [[val0]], [[val1]]
  ; CHECK: [[res4:%.*]] = zext i1 [[bres4]] to i32
  %tmp37 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 0
  %tmp38 = load <1 x float>, <1 x float>* %tmp37, align 4
  %tmp39 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 1
  %tmp40 = load <1 x float>, <1 x float>* %tmp39, align 4
  %tmp41 = fcmp fast oeq <1 x float> %tmp38, %tmp40
  %tmp42 = extractelement <1 x i1> %tmp41, i64 0
  %tmp43 = zext i1 %tmp42 to i32

  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <1 x float>, <1 x float>* [[adr1]]
  ; CHECK: [[val1:%.*]] = extractelement <1 x float> [[ld1]], i32 0
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <1 x float>, <1 x float>* [[adr2]]
  ; CHECK: [[val2:%.*]] = extractelement <1 x float> [[ld2]], i32 0
  ; CHECK: [[bres5:%.*]] = fcmp fast une float [[val1]], [[val2]]
  ; CHECK: [[res5:%.*]] = zext i1 [[bres5]] to i32
  %tmp44 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 1
  %tmp45 = load <1 x float>, <1 x float>* %tmp44, align 4
  %tmp46 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 2
  %tmp47 = load <1 x float>, <1 x float>* %tmp46, align 4
  %tmp48 = fcmp fast une <1 x float> %tmp45, %tmp47
  %tmp49 = extractelement <1 x i1> %tmp48, i64 0
  %tmp50 = zext i1 %tmp49 to i32

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <1 x float>, <1 x float>* [[adr2]]
  ; CHECK: [[val2:%.*]] = extractelement <1 x float> [[ld2]], i32 0
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <1 x float>, <1 x float>* [[adr3]]
  ; CHECK: [[val3:%.*]] = extractelement <1 x float> [[ld3]], i32 0
  ; CHECK: [[bres6:%.*]] = fcmp fast olt float [[val2]], [[val3]]
  ; CHECK: [[res6:%.*]] = zext i1 [[bres6]] to i32
  %tmp51 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 2
  %tmp52 = load <1 x float>, <1 x float>* %tmp51, align 4
  %tmp53 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 3
  %tmp54 = load <1 x float>, <1 x float>* %tmp53, align 4
  %tmp55 = fcmp fast olt <1 x float> %tmp52, %tmp54
  %tmp56 = extractelement <1 x i1> %tmp55, i64 0
  %tmp57 = zext i1 %tmp56 to i32

  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <1 x float>, <1 x float>* [[adr3]]
  ; CHECK: [[val3:%.*]] = extractelement <1 x float> [[ld3]], i32 0
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <1 x float>, <1 x float>* [[adr4]]
  ; CHECK: [[val4:%.*]] = extractelement <1 x float> [[ld4]], i32 0
  ; CHECK: [[bres7:%.*]] = fcmp fast ogt float [[val3]], [[val4]]
  ; CHECK: [[res7:%.*]] = zext i1 [[bres7]] to i32
  %tmp58 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 3
  %tmp59 = load <1 x float>, <1 x float>* %tmp58, align 4
  %tmp60 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 4
  %tmp61 = load <1 x float>, <1 x float>* %tmp60, align 4
  %tmp62 = fcmp fast ogt <1 x float> %tmp59, %tmp61
  %tmp63 = extractelement <1 x i1> %tmp62, i64 0
  %tmp64 = zext i1 %tmp63 to i32

  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <1 x float>, <1 x float>* [[adr4]]
  ; CHECK: [[val4:%.*]] = extractelement <1 x float> [[ld4]], i32 0
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <1 x float>, <1 x float>* [[adr5]]
  ; CHECK: [[val5:%.*]] = extractelement <1 x float> [[ld5]], i32 0
  ; CHECK: [[bres8:%.*]] = fcmp fast ole float [[val4]], [[val5]]
  ; CHECK: [[res8:%.*]] = zext i1 [[bres8]] to i32
  %tmp65 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 4
  %tmp66 = load <1 x float>, <1 x float>* %tmp65, align 4
  %tmp67 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 5
  %tmp68 = load <1 x float>, <1 x float>* %tmp67, align 4
  %tmp69 = fcmp fast ole <1 x float> %tmp66, %tmp68
  %tmp70 = extractelement <1 x i1> %tmp69, i64 0
  %tmp71 = zext i1 %tmp70 to i32

  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <1 x float>, <1 x float>* [[adr5]]
  ; CHECK: [[val5:%.*]] = extractelement <1 x float> [[ld5]], i32 0
  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load <1 x float>, <1 x float>* [[adr6]]
  ; CHECK: [[val6:%.*]] = extractelement <1 x float> [[ld6]], i32 0
  ; CHECK: [[bres9:%.*]] = fcmp fast oge float [[val5]], [[val6]]
  ; CHECK: [[res9:%.*]] = zext i1 [[bres9]] to i32
  %tmp72 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 5
  %tmp73 = load <1 x float>, <1 x float>* %tmp72, align 4
  %tmp74 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %consequences, i32 0, i32 6
  %tmp75 = load <1 x float>, <1 x float>* %tmp74, align 4
  %tmp76 = fcmp fast oge <1 x float> %tmp73, %tmp75
  %tmp77 = extractelement <1 x i1> %tmp76, i64 0
  %tmp78 = zext i1 %tmp77 to i32

  %tmp79 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 0
  store i32 %tmp4, i32* %tmp79
  %tmp80 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 1
  store i32 %tmp14, i32* %tmp80
  %tmp81 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 2
  store i32 %tmp24, i32* %tmp81
  %tmp82 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 3
  store i32 %tmp36, i32* %tmp82
  %tmp83 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 4
  store i32 %tmp43, i32* %tmp83
  %tmp84 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 5
  store i32 %tmp50, i32* %tmp84
  %tmp85 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 6
  store i32 %tmp57, i32* %tmp85
  %tmp86 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 7
  store i32 %tmp64, i32* %tmp86
  %tmp87 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 8
  store i32 %tmp71, i32* %tmp87
  %tmp88 = getelementptr inbounds [10 x i32], [10 x i32]* %agg.result, i32 0, i32 9
  store i32 %tmp78, i32* %tmp88
  ret void
}

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?index
define void @"\01?index@@YA$$BY09V?$vector@M$00@@Y09V1@H@Z"([10 x <1 x float>]* noalias sret %agg.result, [10 x <1 x float>]* %things, i32 %i) #0 {
bb:
  ; CHECK: %res.0 = alloca [10 x float]
  %res.0 = alloca [10 x float]

  ; CHECK: [[adr0:%.*]] = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 0
  ; CHECK: store float 0.000000e+00, float* [[adr0]]
  %tmp1 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 0
  store float 0.000000e+00, float* %tmp1

  ; CHECK: [[adri:%.*]] = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 %i
  ; CHECK: store float 1.000000e+00, float* [[adri]]
  %tmp2 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 %i
  store float 1.000000e+00, float* %tmp2

  ; CHECK: [[adr2:%.*]] = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 2
  ; CHECK: store float 2.000000e+00, float* [[adr2]]
  %tmp3 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 2
  store float 2.000000e+00, float* %tmp3

  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 0
  ; CHECK: [[ld0:%.*]] = load <1 x float>, <1 x float>* [[adr0]]
  ; CHECK: [[adr3:%.*]] = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 3
  ; CHECK: [[val0:%.*]] = extractelement <1 x float> [[ld0]], i64 0
  ; CHECK: store float [[val0]], float* [[adr3]]
  %tmp4 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 0
  %tmp5 = load <1 x float>, <1 x float>* %tmp4, align 4
  %tmp6 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 3
  %tmp7 = extractelement <1 x float> %tmp5, i64 0
  store float %tmp7, float* %tmp6

  ; CHECK: [[adri:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 %i
  ; CHECK: [[ldi:%.*]] = load <1 x float>, <1 x float>* [[adri]]
  ; CHECK: [[adr4:%.*]] = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 4
  ; CHECK: [[vali:%.*]] = extractelement <1 x float> [[ldi]], i64 0
  ; CHECK: store float [[vali]], float* [[adr4]]
  %tmp8 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 %i
  %tmp9 = load <1 x float>, <1 x float>* %tmp8, align 4
  %tmp10 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 4
  %tmp11 = extractelement <1 x float> %tmp9, i64 0
  store float %tmp11, float* %tmp10

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <1 x float>, <1 x float>* [[adr2]]
  ; CHECK: [[adr5:%.*]] = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 5
  ; CHECK: [[val2:%.*]] = extractelement <1 x float> [[ld2]], i64 0
  ; CHECK: store float [[val2]], float* [[adr5]]
  %tmp12 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %things, i32 0, i32 2
  %tmp13 = load <1 x float>, <1 x float>* %tmp12, align 4
  %tmp14 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 5
  %tmp15 = extractelement <1 x float> %tmp13, i64 0
  store float %tmp15, float* %tmp14

  %tmp16 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 0
  %tmp17 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 0
  %load17 = load float, float* %tmp17
  %insert18 = insertelement <1 x float> undef, float %load17, i64 0
  store <1 x float> %insert18, <1 x float>* %tmp16

  %tmp18 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 1
  %tmp19 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 1
  %load15 = load float, float* %tmp19
  %insert16 = insertelement <1 x float> undef, float %load15, i64 0
  store <1 x float> %insert16, <1 x float>* %tmp18

  %tmp20 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 2
  %tmp21 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 2
  %load13 = load float, float* %tmp21
  %insert14 = insertelement <1 x float> undef, float %load13, i64 0
  store <1 x float> %insert14, <1 x float>* %tmp20

  %tmp22 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 3
  %tmp23 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 3
  %load11 = load float, float* %tmp23
  %insert12 = insertelement <1 x float> undef, float %load11, i64 0
  store <1 x float> %insert12, <1 x float>* %tmp22

  %tmp24 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 4
  %tmp25 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 4
  %load9 = load float, float* %tmp25
  %insert10 = insertelement <1 x float> undef, float %load9, i64 0
  store <1 x float> %insert10, <1 x float>* %tmp24

  %tmp26 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 5
  %tmp27 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 5
  %load7 = load float, float* %tmp27
  %insert8 = insertelement <1 x float> undef, float %load7, i64 0
  store <1 x float> %insert8, <1 x float>* %tmp26

  %tmp28 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 6
  %tmp29 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 6
  %load5 = load float, float* %tmp29
  %insert6 = insertelement <1 x float> undef, float %load5, i64 0
  store <1 x float> %insert6, <1 x float>* %tmp28

  %tmp30 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 7
  %tmp31 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 7
  %load3 = load float, float* %tmp31
  %insert4 = insertelement <1 x float> undef, float %load3, i64 0
  store <1 x float> %insert4, <1 x float>* %tmp30

  %tmp32 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 8
  %tmp33 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 8
  %load1 = load float, float* %tmp33
  %insert2 = insertelement <1 x float> undef, float %load1, i64 0
  store <1 x float> %insert2, <1 x float>* %tmp32

  %tmp34 = getelementptr inbounds [10 x <1 x float>], [10 x <1 x float>]* %agg.result, i32 0, i32 9
  %tmp35 = getelementptr [10 x float], [10 x float]* %res.0, i32 0, i32 9
  %load = load float, float* %tmp35
  %insert = insertelement <1 x float> undef, float %load, i64 0
  store <1 x float> %insert, <1 x float>* %tmp34

  ret void
}

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?bittwiddlers
define void @"\01?bittwiddlers@@YAXY0L@$$CAI@Z"([11 x i32]* noalias %things) #0 {
bb:
  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load i32, i32* [[adr1]], align 4
  ; CHECK: [[res0:%.*]] = xor i32 [[ld1]], -1
  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 0
  ; CHECK: store i32 [[res0]], i32* [[adr0]], align 4
  %tmp = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 1
  %tmp1 = load i32, i32* %tmp, align 4
  %tmp2 = xor i32 %tmp1, -1
  %tmp3 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 0
  store i32 %tmp2, i32* %tmp3, align 4

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load i32, i32* [[adr2]], align 4
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load i32, i32* [[adr3]], align 4
  ; CHECK: [[res1:%.*]] = or i32 [[ld2]], [[ld3]]
  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 1
  ; CHECK: store i32 [[res1]], i32* [[adr1]], align 4
  %tmp4 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 2
  %tmp5 = load i32, i32* %tmp4, align 4
  %tmp6 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 3
  %tmp7 = load i32, i32* %tmp6, align 4
  %tmp8 = or i32 %tmp5, %tmp7
  %tmp9 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 1
  store i32 %tmp8, i32* %tmp9, align 4

  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load i32, i32* [[adr3]], align 4
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load i32, i32* [[adr4]], align 4
  ; CHECK: [[res2:%.*]] = and i32 [[ld3]], [[ld4]]
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 2
  ; CHECK: store i32 [[res2]], i32* [[adr2]], align 4
  %tmp10 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 3
  %tmp11 = load i32, i32* %tmp10, align 4
  %tmp12 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 4
  %tmp13 = load i32, i32* %tmp12, align 4
  %tmp14 = and i32 %tmp11, %tmp13
  %tmp15 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 2
  store i32 %tmp14, i32* %tmp15, align 4

  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load i32, i32* [[adr4]], align 4
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load i32, i32* [[adr5]], align 4
  ; CHECK: [[res3:%.*]] = xor i32 [[ld4]], [[ld5]]
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 3
  ; CHECK: store i32 [[res3]], i32* [[adr3]], align 4
  %tmp16 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 4
  %tmp17 = load i32, i32* %tmp16, align 4
  %tmp18 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 5
  %tmp19 = load i32, i32* %tmp18, align 4
  %tmp20 = xor i32 %tmp17, %tmp19
  %tmp21 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 3
  store i32 %tmp20, i32* %tmp21, align 4

  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load i32, i32* [[adr5]], align 4
  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load i32, i32* [[adr6]], align 4
  ; CHECK: [[and4:%.*]] = and i32 [[ld6]], 31
  ; CHECK: [[res4:%.*]] = shl i32 [[ld5]], [[and4]]
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 4
  ; CHECK: store i32 [[res4]], i32* [[adr4]], align 4
  %tmp22 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 5
  %tmp23 = load i32, i32* %tmp22, align 4
  %tmp24 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 6
  %tmp25 = load i32, i32* %tmp24, align 4
  %tmp26 = and i32 %tmp25, 31
  %tmp27 = shl i32 %tmp23, %tmp26
  %tmp28 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 4
  store i32 %tmp27, i32* %tmp28, align 4

  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load i32, i32* [[adr6]], align 4
  ; CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 7
  ; CHECK: [[ld7:%.*]] = load i32, i32* [[adr7]], align 4
  ; CHECK: [[and5:%.*]] = and i32 [[ld7]], 31
  ; CHECK: [[res5:%.*]] = lshr i32 [[ld6]], [[and5]]
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 5
  ; CHECK: store i32 [[res5]], i32* [[adr5]], align 4
  %tmp29 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 6
  %tmp30 = load i32, i32* %tmp29, align 4
  %tmp31 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 7
  %tmp32 = load i32, i32* %tmp31, align 4
  %tmp33 = and i32 %tmp32, 31
  %tmp34 = lshr i32 %tmp30, %tmp33
  %tmp35 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 5
  store i32 %tmp34, i32* %tmp35, align 4

  ; CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 8
  ; CHECK: [[ld8:%.*]] = load i32, i32* [[adr8]], align 4
  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load i32, i32* [[adr6]], align 4
  ; CHECK: [[res6:%.*]] = or i32 [[ld6]], [[ld8]]
  ; CHECK: store i32 [[res6]], i32* [[adr6]], align 4
  %tmp36 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 8
  %tmp37 = load i32, i32* %tmp36, align 4
  %tmp38 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 6
  %tmp39 = load i32, i32* %tmp38, align 4
  %tmp40 = or i32 %tmp39, %tmp37
  store i32 %tmp40, i32* %tmp38, align 4

  ; CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 9
  ; CHECK: [[ld9:%.*]] = load i32, i32* [[adr9]], align 4
  ; CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 7
  ; CHECK: [[ld7:%.*]] = load i32, i32* [[adr7]], align 4
  ; CHECK: [[res7:%.*]] = and i32 [[ld7]], [[ld9]]
  ; CHECK: store i32 [[res7]], i32* [[adr7]], align 4
  %tmp41 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 9
  %tmp42 = load i32, i32* %tmp41, align 4
  %tmp43 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 7
  %tmp44 = load i32, i32* %tmp43, align 4
  %tmp45 = and i32 %tmp44, %tmp42
  store i32 %tmp45, i32* %tmp43, align 4

  ; CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 10
  ; CHECK: [[ld10:%.*]] = load i32, i32* [[adr10]], align 4
  ; CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 8
  ; CHECK: [[ld8:%.*]] = load i32, i32* [[adr8]], align 4
  ; CHECK: [[res8:%.*]] = xor i32 [[ld8]], [[ld10]]
  ; CHECK: store i32 [[res8]], i32* [[adr8]], align 4
  %tmp46 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 10
  %tmp47 = load i32, i32* %tmp46, align 4
  %tmp48 = getelementptr inbounds [11 x i32], [11 x i32]* %things, i32 0, i32 8
  %tmp49 = load i32, i32* %tmp48, align 4
  %tmp50 = xor i32 %tmp49, %tmp47
  store i32 %tmp50, i32* %tmp48, align 4

  ret void
}

declare %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32, %dx.types.Handle, i32, i32, i8, i32) #2
declare %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<vector<float, 1> >"(i32, %"class.RWStructuredBuffer<vector<float, 1> >") #2
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!dx.version = !{!3}
!3 = !{i32 1, i32 9}
