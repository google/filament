; RUN: %dxopt %s -scalarizer -S | FileCheck %s

; Vectors of length greather than 1 should get no changes from scalarizer,
; so this unusual test, verifies that the pass makes no changes at all.
; Still justified because prior to 6.9, many changes would result.
; Compiled mostly for float7 vectors with int7 for the integer specific parts.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWStructuredBuffer<float>" = type { float }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }

@"\01?buf@@3PAV?$RWStructuredBuffer@M@@A" = external global [7 x %"class.RWStructuredBuffer<float>"], align 4
@llvm.used = appending global [1 x i8*] [i8* bitcast ([7 x %"class.RWStructuredBuffer<float>"]* @"\01?buf@@3PAV?$RWStructuredBuffer@M@@A" to i8*)], section "llvm.metadata"

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?assignments
define void @"\01?assignments@@YAXY09$$CAV?$vector@M$06@@@Z"([10 x <7 x float>]* noalias %things) #0 {
bb:
  %tmp = load %"class.RWStructuredBuffer<float>", %"class.RWStructuredBuffer<float>"* getelementptr inbounds ([7 x %"class.RWStructuredBuffer<float>"], [7 x %"class.RWStructuredBuffer<float>"]* @"\01?buf@@3PAV?$RWStructuredBuffer@M@@A", i32 0, i32 0)
  %tmp1 = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<float>"(i32 160, %"class.RWStructuredBuffer<float>" %tmp)
  %tmp2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4108, i32 4 })

  ; CHECK: [[buf:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %{{.*}}, i32 1, i32 0, i8 1, i32 4)
  ; CHECK: [[val:%.*]] = extractvalue %dx.types.ResRet.f32 [[buf]], 0
  ; CHECK: [[vec:%.*]] = insertelement <7 x float> undef, float [[val]], i32 0
  ; CHECK: [[res0:%.*]] = shufflevector <7 x float> [[vec]], <7 x float> undef, <7 x i32> zeroinitializer
  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 0
  ; CHECK: store <7 x float> [[res0]], <7 x float>* [[adr0]], align 4
  %RawBufferLoad = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp2, i32 1, i32 0, i8 1, i32 4)
  %tmp3 = extractvalue %dx.types.ResRet.f32 %RawBufferLoad, 0
  %tmp4 = insertelement <7 x float> undef, float %tmp3, i32 0
  %tmp5 = shufflevector <7 x float> %tmp4, <7 x float> undef, <7 x i32> zeroinitializer
  %tmp6 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 0
  store <7 x float> %tmp5, <7 x float>* %tmp6, align 4

  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <7 x float>, <7 x float>* [[adr5]], align 4
  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <7 x float>, <7 x float>* [[adr1]], align 4
  ; CHECK: [[res1:%.*]] = fadd fast <7 x float> [[ld1]], [[ld5]]
  ; CHECK: store <7 x float> [[res1]], <7 x float>* [[adr1]], align 4
  %tmp7 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 5
  %tmp8 = load <7 x float>, <7 x float>* %tmp7, align 4
  %tmp9 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 1
  %tmp10 = load <7 x float>, <7 x float>* %tmp9, align 4
  %tmp11 = fadd fast <7 x float> %tmp10, %tmp8
  store <7 x float> %tmp11, <7 x float>* %tmp9, align 4

  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load <7 x float>, <7 x float>* [[adr6]], align 4
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <7 x float>, <7 x float>* [[adr2]], align 4
  ; CHECK: [[res2:%.*]] = fsub fast <7 x float> [[ld2]], [[ld6]]
  ; CHECK: store <7 x float> [[res2]], <7 x float>* [[adr2]], align 4
  %tmp12 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 6
  %tmp13 = load <7 x float>, <7 x float>* %tmp12, align 4
  %tmp14 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 2
  %tmp15 = load <7 x float>, <7 x float>* %tmp14, align 4
  %tmp16 = fsub fast <7 x float> %tmp15, %tmp13
  store <7 x float> %tmp16, <7 x float>* %tmp14, align 4

  ; CHECK: [[adr7:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 7
  ; CHECK: [[ld7:%.*]] = load <7 x float>, <7 x float>* [[adr7]], align 4
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <7 x float>, <7 x float>* [[adr3]], align 4
  ; CHECK: [[res3:%.*]] = fmul fast <7 x float> [[ld3]], [[ld7]]
  ; CHECK: store <7 x float> [[res3]], <7 x float>* [[adr3]], align 4
  %tmp17 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 7
  %tmp18 = load <7 x float>, <7 x float>* %tmp17, align 4
  %tmp19 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 3
  %tmp20 = load <7 x float>, <7 x float>* %tmp19, align 4
  %tmp21 = fmul fast <7 x float> %tmp20, %tmp18
  store <7 x float> %tmp21, <7 x float>* %tmp19, align 4

  ; CHECK: [[adr8:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 8
  ; CHECK: [[ld8:%.*]] = load <7 x float>, <7 x float>* [[adr8]], align 4
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <7 x float>, <7 x float>* [[adr4]], align 4
  ; CHECK: [[res4:%.*]] = fdiv fast <7 x float> [[ld4]], [[ld8]]
  ; CHECK: store <7 x float> [[res4]], <7 x float>* [[adr4]], align 4
  %tmp22 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 8
  %tmp23 = load <7 x float>, <7 x float>* %tmp22, align 4
  %tmp24 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 4
  %tmp25 = load <7 x float>, <7 x float>* %tmp24, align 4
  %tmp26 = fdiv fast <7 x float> %tmp25, %tmp23
  store <7 x float> %tmp26, <7 x float>* %tmp24, align 4

  ; CHECK: [[adr9:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 9
  ; CHECK: [[ld9:%.*]] = load <7 x float>, <7 x float>* [[adr9]], align 4
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <7 x float>, <7 x float>* [[adr5]], align 4
  ; CHECK: [[res5:%.*]] = frem fast <7 x float> [[ld5]], [[ld9]]
  ; CHECK: store <7 x float> [[res5]], <7 x float>* [[adr5]], align 4
  %tmp27 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 9
  %tmp28 = load <7 x float>, <7 x float>* %tmp27, align 4
  %tmp29 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 5
  %tmp30 = load <7 x float>, <7 x float>* %tmp29, align 4
  %tmp31 = frem fast <7 x float> %tmp30, %tmp28
  store <7 x float> %tmp31, <7 x float>* %tmp29, align 4

  ret void
}

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?arithmetic
define void @"\01?arithmetic@@YA$$BY0L@V?$vector@M$06@@Y0L@$$CAV1@@Z"([11 x <7 x float>]* noalias sret %agg.result, [11 x <7 x float>]* noalias %things) #0 {
bb:
  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 0
  ; CHECK: [[ld0:%.*]] = load <7 x float>, <7 x float>* [[adr0]], align 4
  ; CHECK: [[res0:%.*]] = fsub fast <7 x float> <float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00>, [[ld0]]
  %tmp = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 0
  %tmp1 = load <7 x float>, <7 x float>* %tmp, align 4
  %tmp2 = fsub fast <7 x float> <float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00>, %tmp1

  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 0
  ; CHECK: [[res1:%.*]] = load <7 x float>, <7 x float>* [[adr0]], align 4
  %tmp3 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 0
  %tmp4 = load <7 x float>, <7 x float>* %tmp3, align 4

  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <7 x float>, <7 x float>* [[adr1]], align 4
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <7 x float>, <7 x float>* [[adr2]], align 4
  ; CHECK: [[res2:%.*]] = fadd fast <7 x float> [[ld1]], [[ld2]]
  %tmp5 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 1
  %tmp6 = load <7 x float>, <7 x float>* %tmp5, align 4
  %tmp7 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 2
  %tmp8 = load <7 x float>, <7 x float>* %tmp7, align 4
  %tmp9 = fadd fast <7 x float> %tmp6, %tmp8

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <7 x float>, <7 x float>* [[adr2]], align 4
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <7 x float>, <7 x float>* [[adr3]], align 4
  ; CHECK: [[res3:%.*]] = fsub fast <7 x float> [[ld2]], [[ld3]]
  %tmp10 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 2
  %tmp11 = load <7 x float>, <7 x float>* %tmp10, align 4
  %tmp12 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 3
  %tmp13 = load <7 x float>, <7 x float>* %tmp12, align 4
  %tmp14 = fsub fast <7 x float> %tmp11, %tmp13

  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <7 x float>, <7 x float>* [[adr3]], align 4
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <7 x float>, <7 x float>* [[adr4]], align 4
  ; CHECK: [[res4:%.*]] = fmul fast <7 x float> [[ld3]], [[ld4]]
  %tmp15 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 3
  %tmp16 = load <7 x float>, <7 x float>* %tmp15, align 4
  %tmp17 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 4
  %tmp18 = load <7 x float>, <7 x float>* %tmp17, align 4
  %tmp19 = fmul fast <7 x float> %tmp16, %tmp18

  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <7 x float>, <7 x float>* [[adr4]], align 4
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <7 x float>, <7 x float>* [[adr5]], align 4
  ; CHECK: [[res5:%.*]] = fdiv fast <7 x float> [[ld4]], [[ld5]]
  %tmp20 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 4
  %tmp21 = load <7 x float>, <7 x float>* %tmp20, align 4
  %tmp22 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 5
  %tmp23 = load <7 x float>, <7 x float>* %tmp22, align 4
  %tmp24 = fdiv fast <7 x float> %tmp21, %tmp23

  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <7 x float>, <7 x float>* [[adr5]], align 4
  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load <7 x float>, <7 x float>* [[adr6]], align 4
  ; CHECK: [[res6:%.*]] = frem fast <7 x float> [[ld5]], [[ld6]]
  %tmp25 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 5
  %tmp26 = load <7 x float>, <7 x float>* %tmp25, align 4
  %tmp27 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 6
  %tmp28 = load <7 x float>, <7 x float>* %tmp27, align 4
  %tmp29 = frem fast <7 x float> %tmp26, %tmp28

  ; CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 7
  ; CHECK: [[ld7:%.*]] = load <7 x float>, <7 x float>* [[adr7]], align 4
  ; CHECK: [[res7:%.*]] = fadd fast <7 x float> [[ld7]], <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  ; CHECK: store <7 x float> [[res7]], <7 x float>* [[adr7]], align 4
  %tmp30 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 7
  %tmp31 = load <7 x float>, <7 x float>* %tmp30, align 4
  %tmp32 = fadd fast <7 x float> %tmp31, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  store <7 x float> %tmp32, <7 x float>* %tmp30, align 4

  ; CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 8
  ; CHECK: [[ld8:%.*]] = load <7 x float>, <7 x float>* [[adr8]], align 4
  ; CHECK: [[res8:%.*]] = fadd fast <7 x float> [[ld8]], <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
  ; CHECK: store <7 x float> [[res8]], <7 x float>* [[adr8]], align 4
  %tmp33 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 8
  %tmp34 = load <7 x float>, <7 x float>* %tmp33, align 4
  %tmp35 = fadd fast <7 x float> %tmp34, <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
  store <7 x float> %tmp35, <7 x float>* %tmp33, align 4

  ; CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 9
  ; CHECK: [[ld9:%.*]] = load <7 x float>, <7 x float>* [[adr9]], align 4
  ; CHECK: [[res9:%.*]] = fadd fast <7 x float> [[ld9]], <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  ; CHECK: store <7 x float> [[res9]], <7 x float>* [[adr9]], align 4
  %tmp36 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 9
  %tmp37 = load <7 x float>, <7 x float>* %tmp36, align 4
  %tmp38 = fadd fast <7 x float> %tmp37, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  store <7 x float> %tmp38, <7 x float>* %tmp36, align 4

  ; CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 10
  ; CHECK: [[ld10:%.*]] = load <7 x float>, <7 x float>* [[adr10]], align 4
  ; CHECK: [[res10:%.*]] = fadd fast <7 x float> [[ld10]], <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
  ; CHECK: store <7 x float> [[res10]], <7 x float>* [[adr10]], align 4
  %tmp39 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %things, i32 0, i32 10
  %tmp40 = load <7 x float>, <7 x float>* %tmp39, align 4
  %tmp41 = fadd fast <7 x float> %tmp40, <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
  store <7 x float> %tmp41, <7 x float>* %tmp39, align 4

  %tmp42 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 0
  store <7 x float> %tmp2, <7 x float>* %tmp42
  %tmp43 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 1
  store <7 x float> %tmp4, <7 x float>* %tmp43
  %tmp44 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 2
  store <7 x float> %tmp9, <7 x float>* %tmp44
  %tmp45 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 3
  store <7 x float> %tmp14, <7 x float>* %tmp45
  %tmp46 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 4
  store <7 x float> %tmp19, <7 x float>* %tmp46
  %tmp47 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 5
  store <7 x float> %tmp24, <7 x float>* %tmp47
  %tmp48 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 6
  store <7 x float> %tmp29, <7 x float>* %tmp48
  %tmp49 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 7
  store <7 x float> %tmp31, <7 x float>* %tmp49
  %tmp50 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 8
  store <7 x float> %tmp34, <7 x float>* %tmp50
  %tmp51 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 9
  store <7 x float> %tmp38, <7 x float>* %tmp51
  %tmp52 = getelementptr inbounds [11 x <7 x float>], [11 x <7 x float>]* %agg.result, i32 0, i32 10
  store <7 x float> %tmp41, <7 x float>* %tmp52
  ret void
}

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?logic
define void @"\01?logic@@YA$$BY09V?$vector@_N$06@@Y09V1@Y09V?$vector@M$06@@@Z"([10 x <7 x i32>]* noalias sret %agg.result, [10 x <7 x i32>]* %truth, [10 x <7 x float>]* %consequences) #0 {
bb:
  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 0
  ; CHECK: [[ld0:%.*]] = load <7 x i32>, <7 x i32>* [[adr0]], align 4
  ; CHECK: [[nres0:%.*]] = icmp ne <7 x i32> [[ld0]], zeroinitializer
  ; CHECK: [[bres0:%.*]] = icmp eq <7 x i1> [[nres0:%.*]], zeroinitializer
  ; CHECK: [[res0:%.*]] = zext <7 x i1> [[bres0]] to <7 x i32>
  %tmp = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 0
  %tmp1 = load <7 x i32>, <7 x i32>* %tmp, align 4
  %tmp2 = icmp ne <7 x i32> %tmp1, zeroinitializer
  %tmp3 = icmp eq <7 x i1> %tmp2, zeroinitializer
  %tmp4 = zext <7 x i1> %tmp3 to <7 x i32>

  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <7 x i32>, <7 x i32>* [[adr1]], align 4
  ; CHECK: [[bld1:%.*]] = icmp ne <7 x i32> [[ld1]], zeroinitializer
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <7 x i32>, <7 x i32>* [[adr2]], align 4
  ; CHECK: [[bld2:%.*]] = icmp ne <7 x i32> [[ld2]], zeroinitializer
  ; CHECK: [[bres1:%.*]] = or <7 x i1> [[bld1]], [[bld2]]
  ; CHECK: [[res1:%.*]] = zext <7 x i1> [[bres1]] to <7 x i32>
  %tmp5 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 1
  %tmp6 = load <7 x i32>, <7 x i32>* %tmp5, align 4
  %tmp7 = icmp ne <7 x i32> %tmp6, zeroinitializer
  %tmp8 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 2
  %tmp9 = load <7 x i32>, <7 x i32>* %tmp8, align 4
  %tmp10 = icmp ne <7 x i32> %tmp9, zeroinitializer
  %tmp11 = or <7 x i1> %tmp7, %tmp10
  %tmp12 = zext <7 x i1> %tmp11 to <7 x i32>

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <7 x i32>, <7 x i32>* [[adr2]], align 4
  ; CHECK: [[bld2:%.*]] = icmp ne <7 x i32> [[ld2]], zeroinitializer
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <7 x i32>, <7 x i32>* [[adr3]], align 4
  ; CHECK: [[bld3:%.*]] = icmp ne <7 x i32> [[ld3]], zeroinitializer
  ; CHECK: [[bres2:%.*]] = and <7 x i1> [[bld2]], [[bld3]]
  ; CHECK: [[res2:%.*]] = zext <7 x i1> [[bres2]] to <7 x i32>
  %tmp13 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 2
  %tmp14 = load <7 x i32>, <7 x i32>* %tmp13, align 4
  %tmp15 = icmp ne <7 x i32> %tmp14, zeroinitializer
  %tmp16 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 3
  %tmp17 = load <7 x i32>, <7 x i32>* %tmp16, align 4
  %tmp18 = icmp ne <7 x i32> %tmp17, zeroinitializer
  %tmp19 = and <7 x i1> %tmp15, %tmp18
  %tmp20 = zext <7 x i1> %tmp19 to <7 x i32>

  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <7 x i32>, <7 x i32>* [[adr3]], align 4
  ; CHECK: [[bld3:%.*]] = icmp ne <7 x i32> [[ld3]], zeroinitializer
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <7 x i32>, <7 x i32>* [[adr4]], align 4
  ; CHECK: [[bld4:%.*]] = icmp ne <7 x i32> [[ld4]], zeroinitializer
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <7 x i32>, <7 x i32>* [[adr5]], align 4
  ; CHECK: [[bld5:%.*]] = icmp ne <7 x i32> [[ld5]], zeroinitializer
  ; CHECK: [[bres3:%.*]] = select <7 x i1> [[bld3]], <7 x i1> [[bld4]], <7 x i1> [[bld5]]
  ; CHECK: [[res3:%.*]] = zext <7 x i1> [[bres3]] to <7 x i32>
  %tmp21 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 3
  %tmp22 = load <7 x i32>, <7 x i32>* %tmp21, align 4
  %tmp23 = icmp ne <7 x i32> %tmp22, zeroinitializer
  %tmp24 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 4
  %tmp25 = load <7 x i32>, <7 x i32>* %tmp24, align 4
  %tmp26 = icmp ne <7 x i32> %tmp25, zeroinitializer
  %tmp27 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %truth, i32 0, i32 5
  %tmp28 = load <7 x i32>, <7 x i32>* %tmp27, align 4
  %tmp29 = icmp ne <7 x i32> %tmp28, zeroinitializer
  %tmp30 = select <7 x i1> %tmp23, <7 x i1> %tmp26, <7 x i1> %tmp29
  %tmp31 = zext <7 x i1> %tmp30 to <7 x i32>

  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 0
  ; CHECK: [[ld0:%.*]] = load <7 x float>, <7 x float>* [[adr0]], align 4
  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <7 x float>, <7 x float>* [[adr1]], align 4
  ; CHECK: [[bres1:%.*]] = fcmp fast oeq <7 x float> [[ld0]], [[ld1]]
  ; CHECK: [[res1:%.*]] = zext <7 x i1> [[bres1]] to <7 x i32>
  %tmp32 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 0
  %tmp33 = load <7 x float>, <7 x float>* %tmp32, align 4
  %tmp34 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 1
  %tmp35 = load <7 x float>, <7 x float>* %tmp34, align 4
  %tmp36 = fcmp fast oeq <7 x float> %tmp33, %tmp35
  %tmp37 = zext <7 x i1> %tmp36 to <7 x i32>

  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <7 x float>, <7 x float>* [[adr1]], align 4
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <7 x float>, <7 x float>* [[adr2]], align 4
  ; CHECK: [[bres2:%.*]] = fcmp fast une <7 x float> [[ld1]], [[ld2]]
  ; CHECK: [[res2:%.*]] = zext <7 x i1> [[bres2]] to <7 x i32>
  %tmp38 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 1
  %tmp39 = load <7 x float>, <7 x float>* %tmp38, align 4
  %tmp40 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 2
  %tmp41 = load <7 x float>, <7 x float>* %tmp40, align 4
  %tmp42 = fcmp fast une <7 x float> %tmp39, %tmp41
  %tmp43 = zext <7 x i1> %tmp42 to <7 x i32>

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <7 x float>, <7 x float>* [[adr2]], align 4
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <7 x float>, <7 x float>* [[adr3]], align 4
  ; CHECK: [[bres3:%.*]] = fcmp fast olt <7 x float> [[ld2]], [[ld3]]
  ; CHECK: [[res3:%.*]] = zext <7 x i1> [[bres3]] to <7 x i32>
  %tmp44 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 2
  %tmp45 = load <7 x float>, <7 x float>* %tmp44, align 4
  %tmp46 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 3
  %tmp47 = load <7 x float>, <7 x float>* %tmp46, align 4
  %tmp48 = fcmp fast olt <7 x float> %tmp45, %tmp47
  %tmp49 = zext <7 x i1> %tmp48 to <7 x i32>

  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <7 x float>, <7 x float>* [[adr3]], align 4
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <7 x float>, <7 x float>* [[adr4]], align 4
  ; CHECK: [[bres4:%.*]] = fcmp fast ogt <7 x float> [[ld3]], [[ld4]]
  ; CHECK: [[res4:%.*]] = zext <7 x i1> [[bres4]] to <7 x i32>
  %tmp50 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 3
  %tmp51 = load <7 x float>, <7 x float>* %tmp50, align 4
  %tmp52 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 4
  %tmp53 = load <7 x float>, <7 x float>* %tmp52, align 4
  %tmp54 = fcmp fast ogt <7 x float> %tmp51, %tmp53
  %tmp55 = zext <7 x i1> %tmp54 to <7 x i32>

  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <7 x float>, <7 x float>* [[adr4]], align 4
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <7 x float>, <7 x float>* [[adr5]], align 4
  ; CHECK: [[bres5:%.*]] = fcmp fast ole <7 x float> [[ld4]], [[ld5]]
  ; CHECK: [[res5:%.*]] = zext <7 x i1> [[bres5]] to <7 x i32>
  %tmp56 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 4
  %tmp57 = load <7 x float>, <7 x float>* %tmp56, align 4
  %tmp58 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 5
  %tmp59 = load <7 x float>, <7 x float>* %tmp58, align 4
  %tmp60 = fcmp fast ole <7 x float> %tmp57, %tmp59
  %tmp61 = zext <7 x i1> %tmp60 to <7 x i32>

  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <7 x float>, <7 x float>* [[adr5]], align 4
  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load <7 x float>, <7 x float>* [[adr6]], align 4
  ; CHECK: [[bres6:%.*]] = fcmp fast oge <7 x float> [[ld5]], [[ld6]]
  ; CHECK: [[res6:%.*]] = zext <7 x i1> [[bres6]] to <7 x i32>
  %tmp62 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 5
  %tmp63 = load <7 x float>, <7 x float>* %tmp62, align 4
  %tmp64 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %consequences, i32 0, i32 6
  %tmp65 = load <7 x float>, <7 x float>* %tmp64, align 4
  %tmp66 = fcmp fast oge <7 x float> %tmp63, %tmp65
  %tmp67 = zext <7 x i1> %tmp66 to <7 x i32>

  %tmp68 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 0
  store <7 x i32> %tmp4, <7 x i32>* %tmp68
  %tmp69 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 1
  store <7 x i32> %tmp12, <7 x i32>* %tmp69
  %tmp70 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 2
  store <7 x i32> %tmp20, <7 x i32>* %tmp70
  %tmp71 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 3
  store <7 x i32> %tmp31, <7 x i32>* %tmp71
  %tmp72 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 4
  store <7 x i32> %tmp37, <7 x i32>* %tmp72
  %tmp73 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 5
  store <7 x i32> %tmp43, <7 x i32>* %tmp73
  %tmp74 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 6
  store <7 x i32> %tmp49, <7 x i32>* %tmp74
  %tmp75 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 7
  store <7 x i32> %tmp55, <7 x i32>* %tmp75
  %tmp76 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 8
  store <7 x i32> %tmp61, <7 x i32>* %tmp76
  %tmp77 = getelementptr inbounds [10 x <7 x i32>], [10 x <7 x i32>]* %agg.result, i32 0, i32 9
  store <7 x i32> %tmp67, <7 x i32>* %tmp77
  ret void
}

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?index
define void @"\01?index@@YA$$BY09V?$vector@M$06@@Y09V1@H@Z"([10 x <7 x float>]* noalias sret %agg.result, [10 x <7 x float>]* %things, i32 %i) #0 {
bb:
  %res = alloca [10 x <7 x float>], align 4

  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 0
  ; CHECK: store <7 x float> zeroinitializer, <7 x float>* [[adr0]], align 4
  %tmp1 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 0
  store <7 x float> zeroinitializer, <7 x float>* %tmp1, align 4

  ; CHECK: [[adri:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 %i
  ; CHECK: store <7 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <7 x float>* [[adri]], align 4
  %tmp2 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 %i
  store <7 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <7 x float>* %tmp2, align 4

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 2
  ; CHECK: store <7 x float> <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>, <7 x float>* [[adr2]], align 4
  %tmp3 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 2
  store <7 x float> <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>, <7 x float>* %tmp3, align 4

  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 0
  ; CHECK: [[res3:%.*]] = load <7 x float>, <7 x float>* [[adr0]], align 4
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 3
  ; CHECK: store <7 x float> [[res3]], <7 x float>* [[adr3]], align 4
  %tmp4 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 0
  %tmp5 = load <7 x float>, <7 x float>* %tmp4, align 4
  %tmp6 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 3
  store <7 x float> %tmp5, <7 x float>* %tmp6, align 4

  ; CHECK: [[adri:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 %i
  ; CHECK: [[res4:%.*]] = load <7 x float>, <7 x float>* [[adri]], align 4
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 4
  ; CHECK: store <7 x float> [[res4]], <7 x float>* [[adr4]], align 4
  %tmp7 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 %i
  %tmp8 = load <7 x float>, <7 x float>* %tmp7, align 4
  %tmp9 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 4
  store <7 x float> %tmp8, <7 x float>* %tmp9, align 4

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 2
  ; CHECK: [[res5:%.*]] = load <7 x float>, <7 x float>* [[adr2]], align 4
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 5
  ; CHECK: store <7 x float> [[res5]], <7 x float>* [[adr5]], align 4
  %tmp10 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %things, i32 0, i32 2
  %tmp11 = load <7 x float>, <7 x float>* %tmp10, align 4
  %tmp12 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 5
  store <7 x float> %tmp11, <7 x float>* %tmp12, align 4

  %tmp13 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 0
  %tmp14 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 0
  %tmp15 = load <7 x float>, <7 x float>* %tmp14
  store <7 x float> %tmp15, <7 x float>* %tmp13

  %tmp16 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 1
  %tmp17 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 1
  %tmp18 = load <7 x float>, <7 x float>* %tmp17
  store <7 x float> %tmp18, <7 x float>* %tmp16

  %tmp19 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 2
  %tmp20 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 2
  %tmp21 = load <7 x float>, <7 x float>* %tmp20
  store <7 x float> %tmp21, <7 x float>* %tmp19

  %tmp22 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 3
  %tmp23 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 3
  %tmp24 = load <7 x float>, <7 x float>* %tmp23
  store <7 x float> %tmp24, <7 x float>* %tmp22

  %tmp25 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 4
  %tmp26 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 4
  %tmp27 = load <7 x float>, <7 x float>* %tmp26
  store <7 x float> %tmp27, <7 x float>* %tmp25

  %tmp28 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 5
  %tmp29 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 5
  %tmp30 = load <7 x float>, <7 x float>* %tmp29
  store <7 x float> %tmp30, <7 x float>* %tmp28

  %tmp31 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 6
  %tmp32 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 6
  %tmp33 = load <7 x float>, <7 x float>* %tmp32
  store <7 x float> %tmp33, <7 x float>* %tmp31

  %tmp34 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 7
  %tmp35 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 7
  %tmp36 = load <7 x float>, <7 x float>* %tmp35
  store <7 x float> %tmp36, <7 x float>* %tmp34

  %tmp37 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 8
  %tmp38 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 8
  %tmp39 = load <7 x float>, <7 x float>* %tmp38
  store <7 x float> %tmp39, <7 x float>* %tmp37

  %tmp40 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %agg.result, i32 0, i32 9
  %tmp41 = getelementptr inbounds [10 x <7 x float>], [10 x <7 x float>]* %res, i32 0, i32 9
  %tmp42 = load <7 x float>, <7 x float>* %tmp41
  store <7 x float> %tmp42, <7 x float>* %tmp40

  ret void
}

; Function Attrs: nounwind
; CHECK-LABEL: define void @"\01?bittwiddlers
define void @"\01?bittwiddlers@@YAXY0L@$$CAV?$vector@I$06@@@Z"([11 x <7 x i32>]* noalias %things) #0 {
bb:
  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 1
  ; CHECK: [[ld1:%.*]] = load <7 x i32>, <7 x i32>* [[adr1]], align 4
  ; CHECK: [[res0:%.*]] = xor <7 x i32> [[ld1]], <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
  ; CHECK: [[adr0:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 0
  ; CHECK: store <7 x i32> [[res0]], <7 x i32>* [[adr0]], align 4
  %tmp = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 1
  %tmp1 = load <7 x i32>, <7 x i32>* %tmp, align 4
  %tmp2 = xor <7 x i32> %tmp1, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
  %tmp3 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 0
  store <7 x i32> %tmp2, <7 x i32>* %tmp3, align 4

  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 2
  ; CHECK: [[ld2:%.*]] = load <7 x i32>, <7 x i32>* [[adr2]], align 4
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <7 x i32>, <7 x i32>* [[adr3]], align 4
  ; CHECK: [[res1:%.*]] = or <7 x i32> [[ld2]], [[ld3]]
  ; CHECK: [[adr1:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 1
  ; CHECK: store <7 x i32> [[res1]], <7 x i32>* [[adr1]], align 4
  %tmp4 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 2
  %tmp5 = load <7 x i32>, <7 x i32>* %tmp4, align 4
  %tmp6 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 3
  %tmp7 = load <7 x i32>, <7 x i32>* %tmp6, align 4
  %tmp8 = or <7 x i32> %tmp5, %tmp7
  %tmp9 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 1
  store <7 x i32> %tmp8, <7 x i32>* %tmp9, align 4

  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 3
  ; CHECK: [[ld3:%.*]] = load <7 x i32>, <7 x i32>* [[adr3]], align 4
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <7 x i32>, <7 x i32>* [[adr4]], align 4
  ; CHECK: [[res2:%.*]] = and <7 x i32> [[ld3]], [[ld4]]
  ; CHECK: [[adr2:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 2
  ; CHECK: store <7 x i32> [[res2]], <7 x i32>* [[adr2]], align 4
  %tmp10 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 3
  %tmp11 = load <7 x i32>, <7 x i32>* %tmp10, align 4
  %tmp12 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 4
  %tmp13 = load <7 x i32>, <7 x i32>* %tmp12, align 4
  %tmp14 = and <7 x i32> %tmp11, %tmp13
  %tmp15 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 2
  store <7 x i32> %tmp14, <7 x i32>* %tmp15, align 4

  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 4
  ; CHECK: [[ld4:%.*]] = load <7 x i32>, <7 x i32>* [[adr4]], align 4
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <7 x i32>, <7 x i32>* [[adr5]], align 4
  ; CHECK: [[res3:%.*]] = xor <7 x i32> [[ld4]], [[ld5]]
  ; CHECK: [[adr3:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 3
  ; CHECK: store <7 x i32> [[res3]], <7 x i32>* [[adr3]], align 4
  %tmp16 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 4
  %tmp17 = load <7 x i32>, <7 x i32>* %tmp16, align 4
  %tmp18 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 5
  %tmp19 = load <7 x i32>, <7 x i32>* %tmp18, align 4
  %tmp20 = xor <7 x i32> %tmp17, %tmp19
  %tmp21 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 3
  store <7 x i32> %tmp20, <7 x i32>* %tmp21, align 4

  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 5
  ; CHECK: [[ld5:%.*]] = load <7 x i32>, <7 x i32>* [[adr5]], align 4
  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load <7 x i32>, <7 x i32>* [[adr6]], align 4
  ; CHECK: [[shv6:%.*]] = and <7 x i32> [[ld6]], <i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31>
  ; CHECK: [[res4:%.*]] = shl <7 x i32> [[ld5]], [[shv6]]
  ; CHECK: [[adr4:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 4
  ; CHECK: store <7 x i32> [[res4]], <7 x i32>* [[adr4]], align 4
  %tmp22 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 5
  %tmp23 = load <7 x i32>, <7 x i32>* %tmp22, align 4
  %tmp24 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 6
  %tmp25 = load <7 x i32>, <7 x i32>* %tmp24, align 4
  %tmp26 = and <7 x i32> %tmp25, <i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31>
  %tmp27 = shl <7 x i32> %tmp23, %tmp26
  %tmp28 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 4
  store <7 x i32> %tmp27, <7 x i32>* %tmp28, align 4

  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load <7 x i32>, <7 x i32>* [[adr6]], align 4
  ; CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 7
  ; CHECK: [[ld7:%.*]] = load <7 x i32>, <7 x i32>* [[adr7]], align 4
  ; CHECK: [[shv7:%.*]] = and <7 x i32> [[ld7]], <i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31>
  ; CHECK: [[res5:%.*]] = lshr <7 x i32> [[ld6]], [[shv7]]
  ; CHECK: [[adr5:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 5
  ; CHECK: store <7 x i32> [[res5]], <7 x i32>* [[adr5]], align 4
  %tmp29 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 6
  %tmp30 = load <7 x i32>, <7 x i32>* %tmp29, align 4
  %tmp31 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 7
  %tmp32 = load <7 x i32>, <7 x i32>* %tmp31, align 4
  %tmp33 = and <7 x i32> %tmp32, <i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31>
  %tmp34 = lshr <7 x i32> %tmp30, %tmp33
  %tmp35 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 5
  store <7 x i32> %tmp34, <7 x i32>* %tmp35, align 4

  ; CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 8
  ; CHECK: [[ld8:%.*]] = load <7 x i32>, <7 x i32>* [[adr8]], align 4
  ; CHECK: [[adr6:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 6
  ; CHECK: [[ld6:%.*]] = load <7 x i32>, <7 x i32>* [[adr6]], align 4
  ; CHECK: [[res6:%.*]] = or <7 x i32> [[ld6]], [[ld8]]
  ; CHECK: store <7 x i32> [[res6]], <7 x i32>* [[adr6]], align 4
  %tmp36 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 8
  %tmp37 = load <7 x i32>, <7 x i32>* %tmp36, align 4
  %tmp38 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 6
  %tmp39 = load <7 x i32>, <7 x i32>* %tmp38, align 4
  %tmp40 = or <7 x i32> %tmp39, %tmp37
  store <7 x i32> %tmp40, <7 x i32>* %tmp38, align 4

  ; CHECK: [[adr9:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 9
  ; CHECK: [[ld9:%.*]] = load <7 x i32>, <7 x i32>* [[adr9]], align 4
  ; CHECK: [[adr7:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 7
  ; CHECK: [[ld7:%.*]] = load <7 x i32>, <7 x i32>* [[adr7]], align 4
  ; CHECK: [[res7:%.*]] = and <7 x i32> [[ld7]], [[ld9]]
  ; CHECK: store <7 x i32> [[res7]], <7 x i32>* [[adr7]], align 4
  %tmp41 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 9
  %tmp42 = load <7 x i32>, <7 x i32>* %tmp41, align 4
  %tmp43 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 7
  %tmp44 = load <7 x i32>, <7 x i32>* %tmp43, align 4
  %tmp45 = and <7 x i32> %tmp44, %tmp42
  store <7 x i32> %tmp45, <7 x i32>* %tmp43, align 4

  ; CHECK: [[adr10:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 10
  ; CHECK: [[ld10:%.*]] = load <7 x i32>, <7 x i32>* [[adr10]], align 4
  ; CHECK: [[adr8:%.*]] = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 8
  ; CHECK: [[ld8:%.*]] = load <7 x i32>, <7 x i32>* [[adr8]], align 4
  ; CHECK: [[res8:%.*]] = xor <7 x i32> [[ld8]], [[ld10]]
  ; CHECK: store <7 x i32> [[res8]], <7 x i32>* [[adr8]], align 4
  %tmp46 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 10
  %tmp47 = load <7 x i32>, <7 x i32>* %tmp46, align 4
  %tmp48 = getelementptr inbounds [11 x <7 x i32>], [11 x <7 x i32>]* %things, i32 0, i32 8
  %tmp49 = load <7 x i32>, <7 x i32>* %tmp48, align 4
  %tmp50 = xor <7 x i32> %tmp49, %tmp47
  store <7 x i32> %tmp50, <7 x i32>* %tmp48, align 4

  ret void
}

declare %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32, %dx.types.Handle, i32, i32, i8, i32) #1
declare %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<float>"(i32, %"class.RWStructuredBuffer<float>") #1
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }

!dx.version = !{!3}

!3 = !{i32 1, i32 9}
