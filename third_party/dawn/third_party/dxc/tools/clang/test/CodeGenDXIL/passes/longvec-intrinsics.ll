; RUN: %dxopt %s -dxilgen -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWStructuredBuffer<vector<half, 7> >" = type { <7 x half> }
%"class.RWStructuredBuffer<vector<float, 7> >" = type { <7 x float> }
%"class.RWStructuredBuffer<vector<double, 7> >" = type { <7 x double> }
%"class.RWStructuredBuffer<vector<bool, 7> >" = type { <7 x i32> }
%"class.RWStructuredBuffer<vector<unsigned int, 7> >" = type { <7 x i32> }
%"class.RWStructuredBuffer<vector<long long, 7> >" = type { <7 x i64> }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?hBuf@@3V?$RWStructuredBuffer@V?$vector@$f16@$06@@@@A" = external global %"class.RWStructuredBuffer<vector<half, 7> >", align 2
@"\01?fBuf@@3V?$RWStructuredBuffer@V?$vector@M$06@@@@A" = external global %"class.RWStructuredBuffer<vector<float, 7> >", align 4
@"\01?dBuf@@3V?$RWStructuredBuffer@V?$vector@N$06@@@@A" = external global %"class.RWStructuredBuffer<vector<double, 7> >", align 8
@"\01?bBuf@@3V?$RWStructuredBuffer@V?$vector@_N$06@@@@A" = external global %"class.RWStructuredBuffer<vector<bool, 7> >", align 4
@"\01?uBuf@@3V?$RWStructuredBuffer@V?$vector@I$06@@@@A" = external global %"class.RWStructuredBuffer<vector<unsigned int, 7> >", align 4
@"\01?lBuf@@3V?$RWStructuredBuffer@V?$vector@_J$06@@@@A" = external global %"class.RWStructuredBuffer<vector<long long, 7> >", align 8

; CHECK-LABEL: define void @main()
define void @main() #0 {
bb:
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7f32 @dx.op.rawBufferVectorLoad.v7f32(i32 303, %dx.types.Handle {{%.*}}, i32 11, i32 0, i32 4)
  ; CHECK: [[fvec1:%.*]] = extractvalue %dx.types.ResRet.v7f32 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7f32 @dx.op.rawBufferVectorLoad.v7f32(i32 303, %dx.types.Handle {{%.*}}, i32 12, i32 0, i32 4)
  ; CHECK: [[fvec2:%.*]] = extractvalue %dx.types.ResRet.v7f32 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7f32 @dx.op.rawBufferVectorLoad.v7f32(i32 303, %dx.types.Handle {{%.*}}, i32 13, i32 0, i32 4)
  ; CHECK: [[fvec3:%.*]] = extractvalue %dx.types.ResRet.v7f32 [[ld]], 0

  %exp = alloca <7 x float>, align 4
  %tmp = load %"class.RWStructuredBuffer<vector<float, 7> >", %"class.RWStructuredBuffer<vector<float, 7> >"* @"\01?fBuf@@3V?$RWStructuredBuffer@V?$vector@M$06@@@@A" ; line:23 col:30
  %tmp1 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 7> >" %tmp) ; line:23 col:30
  %tmp2 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32 14, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<float, 7> >" zeroinitializer) ; line:23 col:30
  %tmp3 = call <7 x float>* @"dx.hl.subscript.[].rn.<7 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp2, i32 11) ; line:23 col:30
  %tmp4 = load <7 x float>, <7 x float>* %tmp3 ; line:23 col:30
  %tmp5 = load %"class.RWStructuredBuffer<vector<float, 7> >", %"class.RWStructuredBuffer<vector<float, 7> >"* @"\01?fBuf@@3V?$RWStructuredBuffer@V?$vector@M$06@@@@A" ; line:24 col:30
  %tmp6 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 7> >" %tmp5) ; line:24 col:30
  %tmp7 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32 14, %dx.types.Handle %tmp6, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<float, 7> >" zeroinitializer) ; line:24 col:30
  %tmp8 = call <7 x float>* @"dx.hl.subscript.[].rn.<7 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp7, i32 12) ; line:24 col:30
  %tmp9 = load <7 x float>, <7 x float>* %tmp8 ; line:24 col:30
  %tmp10 = load %"class.RWStructuredBuffer<vector<float, 7> >", %"class.RWStructuredBuffer<vector<float, 7> >"* @"\01?fBuf@@3V?$RWStructuredBuffer@V?$vector@M$06@@@@A" ; line:25 col:30
  %tmp11 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 7> >" %tmp10) ; line:25 col:30
  %tmp12 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32 14, %dx.types.Handle %tmp11, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<float, 7> >" zeroinitializer) ; line:25 col:30
  %tmp13 = call <7 x float>* @"dx.hl.subscript.[].rn.<7 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp12, i32 13) ; line:25 col:30
  %tmp14 = load <7 x float>, <7 x float>* %tmp13 ; line:25 col:30

  ;  Clamp operation.
  ; CHECK: [[max:%.*]] = call <7 x float> @dx.op.binary.v7f32(i32 35, <7 x float> [[fvec1]], <7 x float> [[fvec2]])
  ; CHECK: call <7 x float> @dx.op.binary.v7f32(i32 36, <7 x float> [[max]], <7 x float> [[fvec3]])
  %tmp15 = call <7 x float> @"dx.hl.op.rn.<7 x float> (i32, <7 x float>, <7 x float>, <7 x float>)"(i32 119, <7 x float> %tmp4, <7 x float> %tmp9, <7 x float> %tmp14) ; line:29 col:29

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7f16 @dx.op.rawBufferVectorLoad.v7f16(i32 303, %dx.types.Handle {{%.*}}, i32 14, i32 0, i32 2)
  ; CHECK: [[hvec1:%.*]] = extractvalue %dx.types.ResRet.v7f16 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7f16 @dx.op.rawBufferVectorLoad.v7f16(i32 303, %dx.types.Handle {{%.*}}, i32 15, i32 0, i32 2)
  ; CHECK: [[hvec2:%.*]] = extractvalue %dx.types.ResRet.v7f16 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7f16 @dx.op.rawBufferVectorLoad.v7f16(i32 303, %dx.types.Handle {{%.*}}, i32 16, i32 0, i32 2)
  ; CHECK: [[hvec3:%.*]] = extractvalue %dx.types.ResRet.v7f16 [[ld]], 0
  %tmp16 = load %"class.RWStructuredBuffer<vector<half, 7> >", %"class.RWStructuredBuffer<vector<half, 7> >"* @"\01?hBuf@@3V?$RWStructuredBuffer@V?$vector@$f16@$06@@@@A" ; line:37 col:34
  %tmp17 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<half, 7> >" %tmp16) ; line:37 col:34
  %tmp18 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32 14, %dx.types.Handle %tmp17, %dx.types.ResourceProperties { i32 4108, i32 14 }, %"class.RWStructuredBuffer<vector<half, 7> >" zeroinitializer) ; line:37 col:34
  %tmp19 = call <7 x half>* @"dx.hl.subscript.[].rn.<7 x half>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp18, i32 14) ; line:37 col:34
  %tmp20 = load <7 x half>, <7 x half>* %tmp19 ; line:37 col:34
  %tmp21 = load %"class.RWStructuredBuffer<vector<half, 7> >", %"class.RWStructuredBuffer<vector<half, 7> >"* @"\01?hBuf@@3V?$RWStructuredBuffer@V?$vector@$f16@$06@@@@A" ; line:38 col:34
  %tmp22 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<half, 7> >" %tmp21) ; line:38 col:34
  %tmp23 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32 14, %dx.types.Handle %tmp22, %dx.types.ResourceProperties { i32 4108, i32 14 }, %"class.RWStructuredBuffer<vector<half, 7> >" zeroinitializer) ; line:38 col:34
  %tmp24 = call <7 x half>* @"dx.hl.subscript.[].rn.<7 x half>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp23, i32 15) ; line:38 col:34
  %tmp25 = load <7 x half>, <7 x half>* %tmp24 ; line:38 col:34
  %tmp26 = load %"class.RWStructuredBuffer<vector<half, 7> >", %"class.RWStructuredBuffer<vector<half, 7> >"* @"\01?hBuf@@3V?$RWStructuredBuffer@V?$vector@$f16@$06@@@@A" ; line:39 col:34
  %tmp27 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<half, 7> >" %tmp26) ; line:39 col:34
  %tmp28 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32 14, %dx.types.Handle %tmp27, %dx.types.ResourceProperties { i32 4108, i32 14 }, %"class.RWStructuredBuffer<vector<half, 7> >" zeroinitializer) ; line:39 col:34
  %tmp29 = call <7 x half>* @"dx.hl.subscript.[].rn.<7 x half>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp28, i32 16) ; line:39 col:34
  %tmp30 = load <7 x half>, <7 x half>* %tmp29 ; line:39 col:34

  ; Step operation.
  ; CHECK: [[cmp:%.*]] = fcmp fast olt <7 x half> [[hvec2]], [[hvec1]]
  ; CHECK: select <7 x i1> [[cmp]], <7 x half> zeroinitializer, <7 x half> <half 0xH3C00, half 0xH3C00, half 0xH3C00, half 0xH3C00, half 0xH3C00, half 0xH3C00, half 0xH3C00>
  %tmp31 = call <7 x half> @"dx.hl.op.rn.<7 x half> (i32, <7 x half>, <7 x half>)"(i32 192, <7 x half> %tmp20, <7 x half> %tmp25) ; line:43 col:33

  ;  Exp operation.
  ; CHECK: [[mul:%.*]] = fmul fast <7 x float> <float 0x3FF7154760000000, float 0x3FF7154760000000, float 0x3FF7154760000000, float 0x3FF7154760000000, float 0x3FF7154760000000, float 0x3FF7154760000000, float 0x3FF7154760000000>, [[fvec1]]
  ; CHECK call <7 x float> @dx.op.unary.v7f32(i32 21, <7 x float> [[mul]])
  %tmp32 = call <7 x float> @"dx.hl.op.rn.<7 x float> (i32, <7 x float>)"(i32 139, <7 x float> %tmp4) ; line:47 col:11
  %tmp33 = fadd <7 x float> %tmp15, %tmp32 ; line:47 col:8

  ;  Log operation.
  ; CHECK: [[log:%.*]] = call <7 x half> @dx.op.unary.v7f16(i32 23, <7 x half> [[hvec1]])
  ; CHECK: fmul fast <7 x half> <half 0xH398C, half 0xH398C, half 0xH398C, half 0xH398C, half 0xH398C, half 0xH398C, half 0xH398C>, [[log]]
  %tmp34 = call <7 x half> @"dx.hl.op.rn.<7 x half> (i32, <7 x half>)"(i32 159, <7 x half> %tmp20) ; line:51 col:11
  %tmp35 = fadd <7 x half> %tmp31, %tmp34 ; line:51 col:8

  ; Smoothstep operation.
  ; CHECK: [[sub1:%.*]] = fsub fast <7 x float> [[fvec2]], [[fvec1]]
  ; CHECK: [[sub2:%.*]] = fsub fast <7 x float> [[fvec3]], [[fvec1]]
  ; CHECK: [[div:%.*]] = fdiv fast <7 x float> [[sub2]], [[sub1]]
  ; CHECK: [[sat:%.*]] = call <7 x float> @dx.op.unary.v7f32(i32 7, <7 x float> [[div]])
  ; CHECK: [[mul:%.*]] = fmul fast <7 x float> [[sat]], <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  ; CHECK: [[sub:%.*]] = fsub fast <7 x float> <float 3.000000e+00, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00>, [[mul]]
  ; CHECK: [[mul:%.*]] = fmul fast <7 x float> [[sat]], [[sub]]
  ; CHECK: fmul fast <7 x float> %Saturate, [[mul]]
  %tmp36 = call <7 x float> @"dx.hl.op.rn.<7 x float> (i32, <7 x float>, <7 x float>, <7 x float>)"(i32 189, <7 x float> %tmp4, <7 x float> %tmp9, <7 x float> %tmp14) ; line:61 col:11
  %tmp37 = fadd <7 x float> %tmp33, %tmp36 ; line:61 col:8

  ;  Radians operation.
  ; CHECK: fmul fast <7 x float> <float 0x3F91DF46A0000000, float 0x3F91DF46A0000000, float 0x3F91DF46A0000000, float 0x3F91DF46A0000000, float 0x3F91DF46A0000000, float 0x3F91DF46A0000000, float 0x3F91DF46A0000000>, [[fvec3]]
  %tmp38 = call <7 x float> @"dx.hl.op.rn.<7 x float> (i32, <7 x float>)"(i32 176, <7 x float> %tmp14) ; line:66 col:11
  %tmp39 = fadd <7 x float> %tmp37, %tmp38 ; line:66 col:8
  store <7 x float> %tmp14, <7 x float>* %exp, align 4 ; line:77 col:22

  ;  Frexp operation.
  ; CHECK: [[cmp:%.*]] = fcmp fast une <7 x float> [[fvec1]], zeroinitializer
  ; CHECK: [[ext:%.*]] = sext <7 x i1> [[cmp]] to <7 x i32>
  ; CHECK: [[bct:%.*]] = bitcast <7 x float> [[fvec1]] to <7 x i32>
  ; CHECK: [[and:%.*]] = and <7 x i32> [[bct]], <i32 2139095040, i32 2139095040, i32 2139095040, i32 2139095040, i32 2139095040, i32 2139095040, i32 2139095040>
  ; CHECK: [[add:%.*]] = add <7 x i32> [[and]], <i32 -1056964608, i32 -1056964608, i32 -1056964608, i32 -1056964608, i32 -1056964608, i32 -1056964608, i32 -1056964608>
  ; CHECK: [[and:%.*]] = and <7 x i32> [[add]], [[ext]]
  ; CHECK: [[shr:%.*]] = ashr <7 x i32> [[and]], <i32 23, i32 23, i32 23, i32 23, i32 23, i32 23, i32 23>
  ; CHECK: [[i2f:%.*]] = sitofp <7 x i32> [[shr]] to <7 x float>
  ; CHECK: store <7 x float> [[i2f]], <7 x float>* %exp
  ; CHECK: [[and:%.*]] = and <7 x i32> [[bct]], <i32 8388607, i32 8388607, i32 8388607, i32 8388607, i32 8388607, i32 8388607, i32 8388607>
  ; CHECK: [[or:%.*]] = or <7 x i32> [[and]], <i32 1056964608, i32 1056964608, i32 1056964608, i32 1056964608, i32 1056964608, i32 1056964608, i32 1056964608>
  ; CHECK: [[and:%.*]] = and <7 x i32> [[or]], [[ext]]
  ; CHECK: bitcast <7 x i32> [[and]] to <7 x float>
  %tmp41 = call <7 x float> @"dx.hl.op..<7 x float> (i32, <7 x float>, <7 x float>*)"(i32 150, <7 x float> %tmp4, <7 x float>* %exp) ; line:78 col:11
  %tmp42 = fadd <7 x float> %tmp39, %tmp41 ; line:78 col:8
  %tmp43 = load <7 x float>, <7 x float>* %exp, align 4 ; line:79 col:11
  %tmp44 = fadd <7 x float> %tmp42, %tmp43 ; line:79 col:8

  ;  Lerp operation.
  ; CHECK: [[sub:%.*]] = fsub fast <7 x half> [[hvec3]], [[hvec2]]
  ; CHECK: fmul fast <7 x half> [[hvec1]], [[sub]]
  %tmp45 = call <7 x half> @"dx.hl.op.rn.<7 x half> (i32, <7 x half>, <7 x half>, <7 x half>)"(i32 157, <7 x half> %tmp25, <7 x half> %tmp30, <7 x half> %tmp20) ; line:83 col:11
  %tmp46 = fadd <7 x half> %tmp35, %tmp45 ; line:83 col:8

  ;  Fwidth operation.
  ; [[ddx:%.*]] = call <7 x float> @dx.op.unary.v7f32(i32 83, <7 x float> [[fvec1]])
  ; [[fabsx:%.*]] = call <7 x float> @dx.op.unary.v7f32(i32 6, <7 x float> [[ddx]])
  ; [[ddy:%.*]] = call <7 x float> @dx.op.unary.v7f32(i32 84, <7 x float> [[fvec1]])
  ; [[fabsy:%.*]] = call <7 x float> @dx.op.unary.v7f32(i32 6, <7 x float> [[ddy]])
  ; fadd fast <7 x float> [[fabsx]], [[fabsy]]
  %fwidth = call <7 x float> @"dx.hl.op.rn.<7 x float> (i32, <7 x float>)"(i32 151, <7 x float> %tmp4)
  %widsum = fadd <7 x float> %tmp44, %fwidth

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7i32 @dx.op.rawBufferVectorLoad.v7i32(i32 303, %dx.types.Handle {{%.*}}, i32 17, i32 0, i32 4)
  ; CHECK: [[uvec1:%.*]] = extractvalue %dx.types.ResRet.v7i32 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7i32 @dx.op.rawBufferVectorLoad.v7i32(i32 303, %dx.types.Handle {{%.*}}, i32 18, i32 0, i32 4)
  ; CHECK: [[uvec2:%.*]] = extractvalue %dx.types.ResRet.v7i32 [[ld]], 0
  %tmp47 = load %"class.RWStructuredBuffer<vector<unsigned int, 7> >", %"class.RWStructuredBuffer<vector<unsigned int, 7> >"* @"\01?uBuf@@3V?$RWStructuredBuffer@V?$vector@I$06@@@@A" ; line:90 col:29
  %tmp48 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<unsigned int, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<unsigned int, 7> >" %tmp47) ; line:90 col:29
  %tmp49 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<unsigned int, 7> >\22)"(i32 14, %dx.types.Handle %tmp48, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<unsigned int, 7> >" zeroinitializer) ; line:90 col:29
  %tmp50 = call <7 x i32>* @"dx.hl.subscript.[].rn.<7 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp49, i32 17) ; line:90 col:29
  %tmp51 = load <7 x i32>, <7 x i32>* %tmp50 ; line:90 col:29
  %tmp52 = load %"class.RWStructuredBuffer<vector<unsigned int, 7> >", %"class.RWStructuredBuffer<vector<unsigned int, 7> >"* @"\01?uBuf@@3V?$RWStructuredBuffer@V?$vector@I$06@@@@A" ; line:91 col:29
  %tmp53 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<unsigned int, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<unsigned int, 7> >" %tmp52) ; line:91 col:29
  %tmp54 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<unsigned int, 7> >\22)"(i32 14, %dx.types.Handle %tmp53, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<unsigned int, 7> >" zeroinitializer) ; line:91 col:29
  %tmp55 = call <7 x i32>* @"dx.hl.subscript.[].rn.<7 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp54, i32 18) ; line:91 col:29
  %tmp56 = load <7 x i32>, <7 x i32>* %tmp55 ; line:91 col:29

  ; Unsigned int sign operation.
  ; CHECK: [[cmp:%.*]] = icmp ne <7 x i32> [[uvec2]], zeroinitializer
  ; CHECK: zext <7 x i1> [[cmp]] to <7 x i32>
  %tmp57 = call <7 x i32> @"dx.hl.op.rn.<7 x i32> (i32, <7 x i32>)"(i32 355, <7 x i32> %tmp56) ; line:96 col:12

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7i64 @dx.op.rawBufferVectorLoad.v7i64(i32 303, %dx.types.Handle {{%.*}}, i32 19, i32 0, i32 8)
  ; CHECK: [[lvec1:%.*]] = extractvalue %dx.types.ResRet.v7i64 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7i64 @dx.op.rawBufferVectorLoad.v7i64(i32 303, %dx.types.Handle {{%.*}}, i32 20, i32 0, i32 8)
  ; CHECK: [[lvec2:%.*]] = extractvalue %dx.types.ResRet.v7i64 [[ld]], 0
  %tmp58 = load %"class.RWStructuredBuffer<vector<long long, 7> >", %"class.RWStructuredBuffer<vector<long long, 7> >"* @"\01?lBuf@@3V?$RWStructuredBuffer@V?$vector@_J$06@@@@A" ; line:102 col:32
  %tmp59 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<long long, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<long long, 7> >" %tmp58) ; line:102 col:32
  %tmp60 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<long long, 7> >\22)"(i32 14, %dx.types.Handle %tmp59, %dx.types.ResourceProperties { i32 4108, i32 56 }, %"class.RWStructuredBuffer<vector<long long, 7> >" zeroinitializer) ; line:102 col:32
  %tmp61 = call <7 x i64>* @"dx.hl.subscript.[].rn.<7 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp60, i32 19) ; line:102 col:32
  %tmp62 = load <7 x i64>, <7 x i64>* %tmp61 ; line:102 col:32
  %tmp63 = load %"class.RWStructuredBuffer<vector<long long, 7> >", %"class.RWStructuredBuffer<vector<long long, 7> >"* @"\01?lBuf@@3V?$RWStructuredBuffer@V?$vector@_J$06@@@@A" ; line:103 col:32
  %tmp64 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<long long, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<long long, 7> >" %tmp63) ; line:103 col:32
  %tmp65 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<long long, 7> >\22)"(i32 14, %dx.types.Handle %tmp64, %dx.types.ResourceProperties { i32 4108, i32 56 }, %"class.RWStructuredBuffer<vector<long long, 7> >" zeroinitializer) ; line:103 col:32
  %tmp66 = call <7 x i64>* @"dx.hl.subscript.[].rn.<7 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp65, i32 20) ; line:103 col:32
  %tmp67 = load <7 x i64>, <7 x i64>* %tmp66 ; line:103 col:32

  ; Signed int sign operation.
  ; CHECK: [[lt1:%.*]] = icmp slt <7 x i64> zeroinitializer, [[lvec2]]
  ; CHECK: [[lt2:%.*]] = icmp slt <7 x i64> [[lvec2]], zeroinitializer
  ; CHECK: [[ilt1:%.*]] = zext <7 x i1> [[lt1]] to <7 x i32>
  ; CHECK: [[ilt2:%.*]] = zext <7 x i1> [[lt2]] to <7 x i32>
  ; CHECK: sub <7 x i32> [[ilt1]], [[ilt2]]
  %tmp68 = call <7 x i32> @"dx.hl.op.rn.<7 x i32> (i32, <7 x i64>)"(i32 185, <7 x i64> %tmp67) ; line:110 col:12
  %tmp69 = mul <7 x i32> %tmp57, %tmp68 ; line:110 col:9

  ; UnaryBits operations.
  ; CHECK: call <7 x i32> @dx.op.unaryBits.v7i32(i32 31, <7 x i32> [[uvec2]])
  %countbits = call <7 x i32> @"dx.hl.op.rn.<7 x i32> (i32, <7 x i32>)"(i32 123, <7 x i32> %tmp56)
  %ub1 = add <7 x i32> %tmp69, %countbits
  ; CHECK: call <7 x i32> @dx.op.unaryBits.v7i64(i32 32, <7 x i64> [[lvec2]])
  %lobit = call <7 x i32> @"dx.hl.op.rn.<7 x i32> (i32, <7 x i64>)"(i32 145, <7 x i64> %tmp67)
  %ub2 = add <7 x i32> %ub1, %lobit
  ; CHECK: call <7 x i32> @dx.op.unaryBits.v7i32(i32 33, <7 x i32> [[uvec1]])
  %hibit = call <7 x i32> @"dx.hl.op.rn.<7 x i32> (i32, <7 x i32>)"(i32 350, <7 x i32> %tmp51)
  %ub3 = add <7 x i32> %ub2, %hibit

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7i32 @dx.op.rawBufferVectorLoad.v7i32(i32 303, %dx.types.Handle {{%.*}}, i32 21, i32 0, i32 4) 
  ; CHECK: [[vec:%.*]] = extractvalue %dx.types.ResRet.v7i32 [[ld]], 0
  ; CHECK: [[bvec:%.*]] = icmp ne <7 x i32> [[vec]], zeroinitializer
  ; CHECK: [[vec1:%.*]] = zext <7 x i1> [[bvec]] to <7 x i32>
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7i32 @dx.op.rawBufferVectorLoad.v7i32(i32 303, %dx.types.Handle {{%.*}}, i32 22, i32 0, i32 4) 
  ; CHECK: [[vec:%.*]] = extractvalue %dx.types.ResRet.v7i32 [[ld]], 0
  ; CHECK: [[bvec:%.*]] = icmp ne <7 x i32> [[vec]], zeroinitializer
  ; CHECK: [[vec2:%.*]] = zext <7 x i1> [[bvec]] to <7 x i32>
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7i32 @dx.op.rawBufferVectorLoad.v7i32(i32 303, %dx.types.Handle {{%.*}}, i32 23, i32 0, i32 4) 
  ; CHECK: [[vec:%.*]] = extractvalue %dx.types.ResRet.v7i32 [[ld]], 0
  ; CHECK: [[bvec:%.*]] = icmp ne <7 x i32> [[vec]], zeroinitializer
  ; CHECK: [[vec3:%.*]] = zext <7 x i1> [[bvec]] to <7 x i32>
  %tmp70 = load %"class.RWStructuredBuffer<vector<bool, 7> >", %"class.RWStructuredBuffer<vector<bool, 7> >"* @"\01?bBuf@@3V?$RWStructuredBuffer@V?$vector@_N$06@@@@A" ; line:126 col:29
  %tmp71 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<bool, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<bool, 7> >" %tmp70) ; line:126 col:29
  %tmp72 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<bool, 7> >\22)"(i32 14, %dx.types.Handle %tmp71, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<bool, 7> >" zeroinitializer) ; line:126 col:29
  %tmp73 = call <7 x i32>* @"dx.hl.subscript.[].rn.<7 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp72, i32 21) ; line:126 col:29
  %tmp74 = load <7 x i32>, <7 x i32>* %tmp73 ; line:126 col:29
  %tmp75 = icmp ne <7 x i32> %tmp74, zeroinitializer ; line:126 col:29
  %tmp76 = zext <7 x i1> %tmp75 to <7 x i32> ; line:126 col:21
  %tmp77 = load %"class.RWStructuredBuffer<vector<bool, 7> >", %"class.RWStructuredBuffer<vector<bool, 7> >"* @"\01?bBuf@@3V?$RWStructuredBuffer@V?$vector@_N$06@@@@A" ; line:127 col:29
  %tmp78 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<bool, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<bool, 7> >" %tmp77) ; line:127 col:29
  %tmp79 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<bool, 7> >\22)"(i32 14, %dx.types.Handle %tmp78, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<bool, 7> >" zeroinitializer) ; line:127 col:29
  %tmp80 = call <7 x i32>* @"dx.hl.subscript.[].rn.<7 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp79, i32 22) ; line:127 col:29
  %tmp81 = load <7 x i32>, <7 x i32>* %tmp80 ; line:127 col:29
  %tmp82 = icmp ne <7 x i32> %tmp81, zeroinitializer ; line:127 col:29
  %tmp83 = zext <7 x i1> %tmp82 to <7 x i32> ; line:127 col:21
  %tmp84 = load %"class.RWStructuredBuffer<vector<bool, 7> >", %"class.RWStructuredBuffer<vector<bool, 7> >"* @"\01?bBuf@@3V?$RWStructuredBuffer@V?$vector@_N$06@@@@A" ; line:128 col:29
  %tmp85 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<bool, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<bool, 7> >" %tmp84) ; line:128 col:29
  %tmp86 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<bool, 7> >\22)"(i32 14, %dx.types.Handle %tmp85, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<bool, 7> >" zeroinitializer) ; line:128 col:29
  %tmp87 = call <7 x i32>* @"dx.hl.subscript.[].rn.<7 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp86, i32 23) ; line:128 col:29
  %tmp88 = load <7 x i32>, <7 x i32>* %tmp87 ; line:128 col:29
  %tmp89 = icmp ne <7 x i32> %tmp88, zeroinitializer ; line:128 col:29
  %tmp90 = zext <7 x i1> %tmp89 to <7 x i32> ; line:128 col:21


  ; Or() operation.
  ; CHECK: [[bvec2:%.*]] = icmp ne <7 x i32> [[vec2]], zeroinitializer
  ; CHECK: [[bvec1:%.*]] = icmp ne <7 x i32> [[vec1]], zeroinitializer
  ; CHECK: or <7 x i1> [[bvec1]], [[bvec2]]
  %tmp91 = icmp ne <7 x i32> %tmp83, zeroinitializer ; line:133 col:21
  %tmp92 = icmp ne <7 x i32> %tmp76, zeroinitializer ; line:133 col:14
  %tmp93 = call <7 x i1> @"dx.hl.op.rn.<7 x i1> (i32, <7 x i1>, <7 x i1>)"(i32 169, <7 x i1> %tmp92, <7 x i1> %tmp91) ; line:133 col:11
  %tmp94 = zext <7 x i1> %tmp93 to <7 x i32> ; line:133 col:11
  %tmp95 = add <7 x i32> %ub3, %tmp94 ; line:133 col:8

  ; And() operation.
  ; CHECK: [[bvec3:%.*]] = icmp ne <7 x i32> [[vec3]], zeroinitializer
  ; CHECK: [[bvec2:%.*]] = icmp ne <7 x i32> [[vec2]], zeroinitializer
  ; CHECK: and <7 x i1> [[bvec2]], [[bvec3]]
  %tmp96 = icmp ne <7 x i32> %tmp90, zeroinitializer ; line:137 col:22
  %tmp97 = icmp ne <7 x i32> %tmp83, zeroinitializer ; line:137 col:15
  %tmp98 = call <7 x i1> @"dx.hl.op.rn.<7 x i1> (i32, <7 x i1>, <7 x i1>)"(i32 106, <7 x i1> %tmp97, <7 x i1> %tmp96) ; line:137 col:11
  %tmp99 = zext <7 x i1> %tmp98 to <7 x i32> ; line:137 col:11
  %tmp100 = add <7 x i32> %tmp95, %tmp99 ; line:137 col:8

  ; Select() operation.
  ; CHECK: [[bvec3:%.*]] = icmp ne <7 x i32> [[vec3]], zeroinitializer
  ; CHECK: select <7 x i1> [[bvec3]], <7 x i64> [[lvec1]], <7 x i64> [[lvec2]]
  %tmp101 = icmp ne <7 x i32> %tmp90, zeroinitializer ; line:140 col:38
  %tmp102 = call <7 x i64> @"dx.hl.op.rn.<7 x i64> (i32, <7 x i1>, <7 x i64>, <7 x i64>)"(i32 184, <7 x i1> %tmp101, <7 x i64> %tmp62, <7 x i64> %tmp67) ; line:140 col:31

  ; IsNan operation.
  ; CHECK: call <7 x i1> @dx.op.isSpecialFloat.v7f32(i32 8, <7 x float> [[fvec2]])
  %isnan = call <7 x i1> @"dx.hl.op.rn.<7 x i1> (i32, <7 x float>)"(i32 154, <7 x float> %tmp9)
  %inext = zext <7 x i1> %isnan to <7 x i32>
  %inres = add <7 x i32> %tmp100, %inext

  ; Dot operation.
  ; CHECK: [[el1:%.*]] = extractelement <7 x float> [[fvec1]], i64 0
  ; CHECK: [[el2:%.*]] = extractelement <7 x float> [[fvec2]], i64 0
  ; CHECK: [[mul:%.*]] = fmul fast float [[el1]], [[el2]]
  ; CHECK: [[el1:%.*]] = extractelement <7 x float> [[fvec1]], i64 1
  ; CHECK: [[el2:%.*]] = extractelement <7 x float> [[fvec2]], i64 1
  ; CHECK: [[mad1:%.*]] = call float @dx.op.tertiary.f32(i32 46, float [[el1]], float [[el2]], float [[mul]])
  ; CHECK: [[el1:%.*]] = extractelement <7 x float> [[fvec1]], i64 2
  ; CHECK: [[el2:%.*]] = extractelement <7 x float> [[fvec2]], i64 2
  ; CHECK: [[mad2:%.*]] = call float @dx.op.tertiary.f32(i32 46, float [[el1]], float [[el2]], float [[mad1]])
  ; CHECK: [[el1:%.*]] = extractelement <7 x float> [[fvec1]], i64 3
  ; CHECK: [[el2:%.*]] = extractelement <7 x float> [[fvec2]], i64 3
  ; CHECK: [[mad3:%.*]] = call float @dx.op.tertiary.f32(i32 46, float [[el1]], float [[el2]], float [[mad2]])
  ; CHECK: [[el1:%.*]] = extractelement <7 x float> [[fvec1]], i64 4
  ; CHECK: [[el2:%.*]] = extractelement <7 x float> [[fvec2]], i64 4
  ; CHECK: [[mad4:%.*]] = call float @dx.op.tertiary.f32(i32 46, float [[el1]], float [[el2]], float [[mad3]])
  ; CHECK: [[el1:%.*]] = extractelement <7 x float> [[fvec1]], i64 5
  ; CHECK: [[el2:%.*]] = extractelement <7 x float> [[fvec2]], i64 5
  ; CHECK: [[mad5:%.*]] = call float @dx.op.tertiary.f32(i32 46, float [[el1]], float [[el2]], float [[mad4]])
  ; CHECK: [[el1:%.*]] = extractelement <7 x float> [[fvec1]], i64 6
  ; CHECK: [[el2:%.*]] = extractelement <7 x float> [[fvec2]], i64 6
  ; CHECK: call float @dx.op.tertiary.f32(i32 46, float [[el1]], float [[el2]], float [[mad5]])
  %tmp103 = call float @"dx.hl.op.rn.float (i32, <7 x float>, <7 x float>)"(i32 134, <7 x float> %tmp4, <7 x float> %tmp9) ; line:152 col:11
  %tmp104 = insertelement <7 x float> undef, float %tmp103, i32 0 ; line:152 col:11
  %tmp105 = shufflevector <7 x float> %tmp104, <7 x float> undef, <7 x i32> zeroinitializer ; line:152 col:11
  %tmp106 = fadd <7 x float> %widsum, %tmp105 ; line:152 col:8

  ; Atan operation.
  ; CHECK: call <7 x float> @dx.op.unary.v7f32(i32 17, <7 x float> [[fvec1]])
  %tmp107 = call <7 x float> @"dx.hl.op.rn.<7 x float> (i32, <7 x float>)"(i32 116, <7 x float> %tmp4) ; line:155 col:11
  %tmp108 = fadd <7 x float> %tmp106, %tmp107 ; line:155 col:8

  ; Min operation.
  ; CHECK: call <7 x i32> @dx.op.binary.v7i32(i32 40, <7 x i32> [[uvec1]], <7 x i32> [[uvec2]])
  %tmp109 = call <7 x i32> @"dx.hl.op.rn.<7 x i32> (i32, <7 x i32>, <7 x i32>)"(i32 353, <7 x i32> %tmp51, <7 x i32> %tmp56) ; line:158 col:11
  %tmp110 = add <7 x i32> %inres, %tmp109 ; line:158 col:8

  ; Mad operation.
  ; CHECK: call <7 x float> @dx.op.tertiary.v7f32(i32 46, <7 x float> [[fvec1]], <7 x float> [[fvec2]], <7 x float> [[fvec3]])
  %tmp111 = call <7 x float> @"dx.hl.op.rn.<7 x float> (i32, <7 x float>, <7 x float>, <7 x float>)"(i32 162, <7 x float> %tmp4, <7 x float> %tmp9, <7 x float> %tmp14) ; line:161 col:11
  %tmp112 = fadd <7 x float> %tmp108, %tmp111 ; line:161 col:8

  ; Ddx_fine operation.
  ; CHECK: call <7 x half> @dx.op.unary.v7f16(i32 85, <7 x half> [[hvec1]])
  %ddx = call <7 x half> @"dx.hl.op.rn.<7 x half> (i32, <7 x half>)"(i32 127, <7 x half> %tmp20)
  %ddxsum = fadd <7 x half> %tmp46, %ddx

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7f64 @dx.op.rawBufferVectorLoad.v7f64(i32 303, %dx.types.Handle {{%.*}}, i32 24, i32 0, i32 8) 
  ; CHECK: [[dvec1:%.*]] = extractvalue %dx.types.ResRet.v7f64 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7f64 @dx.op.rawBufferVectorLoad.v7f64(i32 303, %dx.types.Handle {{%.*}}, i32 25, i32 0, i32 8) 
  ; CHECK: [[dvec2:%.*]] = extractvalue %dx.types.ResRet.v7f64 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.v7f64 @dx.op.rawBufferVectorLoad.v7f64(i32 303, %dx.types.Handle {{%.*}}, i32 26, i32 0, i32 8) 
  ; CHECK: [[dvec3:%.*]] = extractvalue %dx.types.ResRet.v7f64 [[ld]], 0
  %tmp113 = load %"class.RWStructuredBuffer<vector<double, 7> >", %"class.RWStructuredBuffer<vector<double, 7> >"* @"\01?dBuf@@3V?$RWStructuredBuffer@V?$vector@N$06@@@@A" ; line:169 col:31
  %tmp114 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<double, 7> >" %tmp113) ; line:169 col:31
  %tmp115 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32 14, %dx.types.Handle %tmp114, %dx.types.ResourceProperties { i32 4108, i32 56 }, %"class.RWStructuredBuffer<vector<double, 7> >" zeroinitializer) ; line:169 col:31
  %tmp116 = call <7 x double>* @"dx.hl.subscript.[].rn.<7 x double>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp115, i32 24) ; line:169 col:31
  %tmp117 = load <7 x double>, <7 x double>* %tmp116 ; line:169 col:31
  %tmp118 = load %"class.RWStructuredBuffer<vector<double, 7> >", %"class.RWStructuredBuffer<vector<double, 7> >"* @"\01?dBuf@@3V?$RWStructuredBuffer@V?$vector@N$06@@@@A" ; line:170 col:31
  %tmp119 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<double, 7> >" %tmp118) ; line:170 col:31
  %tmp120 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32 14, %dx.types.Handle %tmp119, %dx.types.ResourceProperties { i32 4108, i32 56 }, %"class.RWStructuredBuffer<vector<double, 7> >" zeroinitializer) ; line:170 col:31
  %tmp121 = call <7 x double>* @"dx.hl.subscript.[].rn.<7 x double>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp120, i32 25) ; line:170 col:31
  %tmp122 = load <7 x double>, <7 x double>* %tmp121 ; line:170 col:31
  %tmp123 = load %"class.RWStructuredBuffer<vector<double, 7> >", %"class.RWStructuredBuffer<vector<double, 7> >"* @"\01?dBuf@@3V?$RWStructuredBuffer@V?$vector@N$06@@@@A" ; line:171 col:31
  %tmp124 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<double, 7> >" %tmp123) ; line:171 col:31
  %tmp125 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32 14, %dx.types.Handle %tmp124, %dx.types.ResourceProperties { i32 4108, i32 56 }, %"class.RWStructuredBuffer<vector<double, 7> >" zeroinitializer) ; line:171 col:31
  %tmp126 = call <7 x double>* @"dx.hl.subscript.[].rn.<7 x double>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp125, i32 26) ; line:171 col:31
  %tmp127 = load <7 x double>, <7 x double>* %tmp126 ; line:171 col:31

  ; FMA operation.
  ; CHECK: call <7 x double> @dx.op.tertiary.v7f64(i32 47, <7 x double> [[dvec1]], <7 x double> [[dvec2]], <7 x double> [[dvec3]])
  %tmp128 = call <7 x double> @"dx.hl.op.rn.<7 x double> (i32, <7 x double>, <7 x double>, <7 x double>)"(i32 147, <7 x double> %tmp117, <7 x double> %tmp122, <7 x double> %tmp127) ; line:174 col:30
  %tmp129 = load %"class.RWStructuredBuffer<vector<half, 7> >", %"class.RWStructuredBuffer<vector<half, 7> >"* @"\01?hBuf@@3V?$RWStructuredBuffer@V?$vector@$f16@$06@@@@A" ; line:176 col:3
  %tmp130 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<half, 7> >" %tmp129) ; line:176 col:3
  %tmp131 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32 14, %dx.types.Handle %tmp130, %dx.types.ResourceProperties { i32 4108, i32 14 }, %"class.RWStructuredBuffer<vector<half, 7> >" zeroinitializer) ; line:176 col:3
  %tmp132 = call <7 x half>* @"dx.hl.subscript.[].rn.<7 x half>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp131, i32 0) ; line:176 col:3
  store <7 x half> %ddxsum, <7 x half>* %tmp132 ; line:176 col:11
  %tmp133 = load %"class.RWStructuredBuffer<vector<float, 7> >", %"class.RWStructuredBuffer<vector<float, 7> >"* @"\01?fBuf@@3V?$RWStructuredBuffer@V?$vector@M$06@@@@A" ; line:177 col:3
  %tmp134 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 7> >" %tmp133) ; line:177 col:3
  %tmp135 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32 14, %dx.types.Handle %tmp134, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<float, 7> >" zeroinitializer) ; line:177 col:3
  %tmp136 = call <7 x float>* @"dx.hl.subscript.[].rn.<7 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp135, i32 0) ; line:177 col:3
  store <7 x float> %tmp112, <7 x float>* %tmp136 ; line:177 col:11
  %tmp137 = load %"class.RWStructuredBuffer<vector<double, 7> >", %"class.RWStructuredBuffer<vector<double, 7> >"* @"\01?dBuf@@3V?$RWStructuredBuffer@V?$vector@N$06@@@@A" ; line:178 col:3
  %tmp138 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<double, 7> >" %tmp137) ; line:178 col:3
  %tmp139 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32 14, %dx.types.Handle %tmp138, %dx.types.ResourceProperties { i32 4108, i32 56 }, %"class.RWStructuredBuffer<vector<double, 7> >" zeroinitializer) ; line:178 col:3
  %tmp140 = call <7 x double>* @"dx.hl.subscript.[].rn.<7 x double>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp139, i32 0) ; line:178 col:3
  store <7 x double> %tmp128, <7 x double>* %tmp140 ; line:178 col:11
  %tmp141 = load %"class.RWStructuredBuffer<vector<unsigned int, 7> >", %"class.RWStructuredBuffer<vector<unsigned int, 7> >"* @"\01?uBuf@@3V?$RWStructuredBuffer@V?$vector@I$06@@@@A" ; line:179 col:3
  %tmp142 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<unsigned int, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<unsigned int, 7> >" %tmp141) ; line:179 col:3
  %tmp143 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<unsigned int, 7> >\22)"(i32 14, %dx.types.Handle %tmp142, %dx.types.ResourceProperties { i32 4108, i32 28 }, %"class.RWStructuredBuffer<vector<unsigned int, 7> >" zeroinitializer) ; line:179 col:3
  %tmp144 = call <7 x i32>* @"dx.hl.subscript.[].rn.<7 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp143, i32 0) ; line:179 col:3
  store <7 x i32> %tmp110, <7 x i32>* %tmp144 ; line:179 col:11
  %tmp145 = load %"class.RWStructuredBuffer<vector<long long, 7> >", %"class.RWStructuredBuffer<vector<long long, 7> >"* @"\01?lBuf@@3V?$RWStructuredBuffer@V?$vector@_J$06@@@@A" ; line:180 col:3
  %tmp146 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<long long, 7> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<long long, 7> >" %tmp145) ; line:180 col:3
  %tmp147 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<long long, 7> >\22)"(i32 14, %dx.types.Handle %tmp146, %dx.types.ResourceProperties { i32 4108, i32 56 }, %"class.RWStructuredBuffer<vector<long long, 7> >" zeroinitializer) ; line:180 col:3
  %tmp148 = call <7 x i64>* @"dx.hl.subscript.[].rn.<7 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp147, i32 0) ; line:180 col:3
  store <7 x i64> %tmp102, <7 x i64>* %tmp148 ; line:180 col:11
  ret void ; line:181 col:1
}

declare <7 x float>* @"dx.hl.subscript.[].rn.<7 x float>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32, %"class.RWStructuredBuffer<vector<float, 7> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 7> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<vector<float, 7> >") #1
declare <7 x float> @"dx.hl.op.rn.<7 x float> (i32, <7 x float>, <7 x float>, <7 x float>)"(i32, <7 x float>, <7 x float>, <7 x float>) #1
declare <7 x half>* @"dx.hl.subscript.[].rn.<7 x half>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32, %"class.RWStructuredBuffer<vector<half, 7> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<half, 7> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<vector<half, 7> >") #1
declare <7 x half> @"dx.hl.op.rn.<7 x half> (i32, <7 x half>, <7 x half>)"(i32, <7 x half>, <7 x half>) #1
declare <7 x float> @"dx.hl.op.rn.<7 x float> (i32, <7 x float>)"(i32, <7 x float>) #1
declare <7 x half> @"dx.hl.op.rn.<7 x half> (i32, <7 x half>)"(i32, <7 x half>) #1
declare <7 x float> @"dx.hl.op..<7 x float> (i32, <7 x float>, <7 x float>*)"(i32, <7 x float>, <7 x float>*) #0
declare <7 x half> @"dx.hl.op.rn.<7 x half> (i32, <7 x half>, <7 x half>, <7 x half>)"(i32, <7 x half>, <7 x half>, <7 x half>) #1
declare <7 x i32>* @"dx.hl.subscript.[].rn.<7 x i32>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<unsigned int, 7> >\22)"(i32, %"class.RWStructuredBuffer<vector<unsigned int, 7> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<unsigned int, 7> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<vector<unsigned int, 7> >") #1
declare <7 x i32> @"dx.hl.op.rn.<7 x i32> (i32, <7 x i32>)"(i32, <7 x i32>) #1
declare <7 x i64>* @"dx.hl.subscript.[].rn.<7 x i64>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<long long, 7> >\22)"(i32, %"class.RWStructuredBuffer<vector<long long, 7> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<long long, 7> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<vector<long long, 7> >") #1
declare <7 x i32> @"dx.hl.op.rn.<7 x i32> (i32, <7 x i64>)"(i32, <7 x i64>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<bool, 7> >\22)"(i32, %"class.RWStructuredBuffer<vector<bool, 7> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<bool, 7> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<vector<bool, 7> >") #1
declare <7 x i1> @"dx.hl.op.rn.<7 x i1> (i32, <7 x i1>, <7 x i1>)"(i32, <7 x i1>, <7 x i1>) #1
declare <7 x i64> @"dx.hl.op.rn.<7 x i64> (i32, <7 x i1>, <7 x i64>, <7 x i64>)"(i32, <7 x i1>, <7 x i64>, <7 x i64>) #1
declare <7 x i1> @"dx.hl.op.rn.<7 x i1> (i32, <7 x float>)"(i32, <7 x float>) #1
declare float @"dx.hl.op.rn.float (i32, <7 x float>, <7 x float>)"(i32, <7 x float>, <7 x float>) #1
declare <7 x i32> @"dx.hl.op.rn.<7 x i32> (i32, <7 x i32>, <7 x i32>)"(i32, <7 x i32>, <7 x i32>) #1
declare <7 x double>* @"dx.hl.subscript.[].rn.<7 x double>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32, %"class.RWStructuredBuffer<vector<double, 7> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<double, 7> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<vector<double, 7> >") #1
declare <7 x double> @"dx.hl.op.rn.<7 x double> (i32, <7 x double>, <7 x double>, <7 x double>)"(i32, <7 x double>, <7 x double>, <7 x double>) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!pauseresume = !{!1}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !36}
!dx.entryPoints = !{!40}
!dx.fnprops = !{!52}
!dx.options = !{!53, !54}

!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!3 = !{i32 1, i32 9}
!4 = !{!"cs", i32 6, i32 9}
!5 = !{i32 0, %"class.RWStructuredBuffer<vector<half, 7> >" undef, !6, %"class.RWStructuredBuffer<vector<float, 7> >" undef, !11, %"class.RWStructuredBuffer<vector<double, 7> >" undef, !16, %"class.RWStructuredBuffer<vector<bool, 7> >" undef, !21, %"class.RWStructuredBuffer<vector<unsigned int, 7> >" undef, !26, %"class.RWStructuredBuffer<vector<long long, 7> >" undef, !31}
!6 = !{i32 14, !7, !8}
!7 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 8, i32 13, i32 7}
!8 = !{i32 0, !9}
!9 = !{!10}
!10 = !{i32 0, <7 x half> undef}
!11 = !{i32 28, !12, !13}
!12 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9, i32 13, i32 7}
!13 = !{i32 0, !14}
!14 = !{!15}
!15 = !{i32 0, <7 x float> undef}
!16 = !{i32 56, !17, !18}
!17 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 10, i32 13, i32 7}
!18 = !{i32 0, !19}
!19 = !{!20}
!20 = !{i32 0, <7 x double> undef}
!21 = !{i32 28, !22, !23}
!22 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 1, i32 13, i32 7}
!23 = !{i32 0, !24}
!24 = !{!25}
!25 = !{i32 0, <7 x i1> undef}
!26 = !{i32 28, !27, !28}
!27 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 5, i32 13, i32 7}
!28 = !{i32 0, !29}
!29 = !{!30}
!30 = !{i32 0, <7 x i32> undef}
!31 = !{i32 56, !32, !33}
!32 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 6, i32 13, i32 7}
!33 = !{i32 0, !34}
!34 = !{!35}
!35 = !{i32 0, <7 x i64> undef}
!36 = !{i32 1, void ()* @main, !37}
!37 = !{!38}
!38 = !{i32 1, !39, !39}
!39 = !{}
!40 = !{void ()* @main, !"main", null, !41, null}
!41 = !{null, !42, null, null}
!42 = !{!43, !45, !47, !49, !50, !51}
!43 = !{i32 0, %"class.RWStructuredBuffer<vector<half, 7> >"* @"\01?hBuf@@3V?$RWStructuredBuffer@V?$vector@$f16@$06@@@@A", !"hBuf", i32 -1, i32 -1, i32 1, i32 12, i1 false, i1 false, i1 false, !44}
!44 = !{i32 1, i32 14}
!45 = !{i32 1, %"class.RWStructuredBuffer<vector<float, 7> >"* @"\01?fBuf@@3V?$RWStructuredBuffer@V?$vector@M$06@@@@A", !"fBuf", i32 -1, i32 -1, i32 1, i32 12, i1 false, i1 false, i1 false, !46}
!46 = !{i32 1, i32 28}
!47 = !{i32 2, %"class.RWStructuredBuffer<vector<double, 7> >"* @"\01?dBuf@@3V?$RWStructuredBuffer@V?$vector@N$06@@@@A", !"dBuf", i32 -1, i32 -1, i32 1, i32 12, i1 false, i1 false, i1 false, !48}
!48 = !{i32 1, i32 56}
!49 = !{i32 3, %"class.RWStructuredBuffer<vector<bool, 7> >"* @"\01?bBuf@@3V?$RWStructuredBuffer@V?$vector@_N$06@@@@A", !"bBuf", i32 -1, i32 -1, i32 1, i32 12, i1 false, i1 false, i1 false, !46}
!50 = !{i32 4, %"class.RWStructuredBuffer<vector<unsigned int, 7> >"* @"\01?uBuf@@3V?$RWStructuredBuffer@V?$vector@I$06@@@@A", !"uBuf", i32 -1, i32 -1, i32 1, i32 12, i1 false, i1 false, i1 false, !46}
!51 = !{i32 5, %"class.RWStructuredBuffer<vector<long long, 7> >"* @"\01?lBuf@@3V?$RWStructuredBuffer@V?$vector@_J$06@@@@A", !"lBuf", i32 -1, i32 -1, i32 1, i32 12, i1 false, i1 false, i1 false, !48}
!52 = !{void ()* @main, i32 5, i32 8, i32 1, i32 1}
!53 = !{i32 0}
!54 = !{i32 -1}
!59 = !{!60, !60, i64 0}
!60 = !{!"omnipotent char", !61, i64 0}
!61 = !{!"Simple C/C++ TBAA"}
