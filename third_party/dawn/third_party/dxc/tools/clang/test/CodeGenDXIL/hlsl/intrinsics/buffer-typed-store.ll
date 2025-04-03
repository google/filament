; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWBuffer<vector<float, 3> >" = type { <3 x float> }
%"class.RWBuffer<vector<bool, 2> >" = type { <2 x i32> }
%"class.RWBuffer<vector<unsigned long long, 2> >" = type { <2 x i64> }
%"class.RWBuffer<double>" = type { double }
%"class.RWTexture1D<vector<float, 3> >" = type { <3 x float> }
%"class.RWTexture1D<vector<bool, 2> >" = type { <2 x i32> }
%"class.RWTexture1D<vector<unsigned long long, 2> >" = type { <2 x i64> }
%"class.RWTexture1D<double>" = type { double }
%"class.RWTexture2D<vector<float, 3> >" = type { <3 x float> }
%"class.RWTexture2D<vector<bool, 2> >" = type { <2 x i32> }
%"class.RWTexture2D<vector<unsigned long long, 2> >" = type { <2 x i64> }
%"class.RWTexture2D<double>" = type { double }
%"class.RWTexture3D<vector<float, 3> >" = type { <3 x float> }
%"class.RWTexture3D<vector<bool, 2> >" = type { <2 x i32> }
%"class.RWTexture3D<vector<unsigned long long, 2> >" = type { <2 x i64> }
%"class.RWTexture3D<double>" = type { double }
%"class.RWTexture2DMS<vector<float, 3>, 0>" = type { <3 x float>, %"class.RWTexture2DMS<vector<float, 3>, 0>::sample_type" }
%"class.RWTexture2DMS<vector<float, 3>, 0>::sample_type" = type { i32 }
%"class.RWTexture2DMS<vector<bool, 2>, 0>" = type { <2 x i32>, %"class.RWTexture2DMS<vector<bool, 2>, 0>::sample_type" }
%"class.RWTexture2DMS<vector<bool, 2>, 0>::sample_type" = type { i32 }
%"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" = type { <2 x i64>, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>::sample_type" }
%"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>::sample_type" = type { i32 }
%"class.RWTexture2DMS<double, 0>" = type { double, %"class.RWTexture2DMS<double, 0>::sample_type" }
%"class.RWTexture2DMS<double, 0>::sample_type" = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?FTyBuf@@3V?$RWBuffer@V?$vector@M$02@@@@A" = external global %"class.RWBuffer<vector<float, 3> >", align 4
@"\01?BTyBuf@@3V?$RWBuffer@V?$vector@_N$01@@@@A" = external global %"class.RWBuffer<vector<bool, 2> >", align 4
@"\01?LTyBuf@@3V?$RWBuffer@V?$vector@_K$01@@@@A" = external global %"class.RWBuffer<vector<unsigned long long, 2> >", align 8
@"\01?DTyBuf@@3V?$RWBuffer@N@@A" = external global %"class.RWBuffer<double>", align 8
@"\01?FTex1d@@3V?$RWTexture1D@V?$vector@M$02@@@@A" = external global %"class.RWTexture1D<vector<float, 3> >", align 4
@"\01?BTex1d@@3V?$RWTexture1D@V?$vector@_N$01@@@@A" = external global %"class.RWTexture1D<vector<bool, 2> >", align 4
@"\01?LTex1d@@3V?$RWTexture1D@V?$vector@_K$01@@@@A" = external global %"class.RWTexture1D<vector<unsigned long long, 2> >", align 8
@"\01?DTex1d@@3V?$RWTexture1D@N@@A" = external global %"class.RWTexture1D<double>", align 8
@"\01?FTex2d@@3V?$RWTexture2D@V?$vector@M$02@@@@A" = external global %"class.RWTexture2D<vector<float, 3> >", align 4
@"\01?BTex2d@@3V?$RWTexture2D@V?$vector@_N$01@@@@A" = external global %"class.RWTexture2D<vector<bool, 2> >", align 4
@"\01?LTex2d@@3V?$RWTexture2D@V?$vector@_K$01@@@@A" = external global %"class.RWTexture2D<vector<unsigned long long, 2> >", align 8
@"\01?DTex2d@@3V?$RWTexture2D@N@@A" = external global %"class.RWTexture2D<double>", align 8
@"\01?FTex3d@@3V?$RWTexture3D@V?$vector@M$02@@@@A" = external global %"class.RWTexture3D<vector<float, 3> >", align 4
@"\01?BTex3d@@3V?$RWTexture3D@V?$vector@_N$01@@@@A" = external global %"class.RWTexture3D<vector<bool, 2> >", align 4
@"\01?LTex3d@@3V?$RWTexture3D@V?$vector@_K$01@@@@A" = external global %"class.RWTexture3D<vector<unsigned long long, 2> >", align 8
@"\01?DTex3d@@3V?$RWTexture3D@N@@A" = external global %"class.RWTexture3D<double>", align 8
@"\01?FTex2dMs@@3V?$RWTexture2DMS@V?$vector@M$02@@$0A@@@A" = external global %"class.RWTexture2DMS<vector<float, 3>, 0>", align 4
@"\01?BTex2dMs@@3V?$RWTexture2DMS@V?$vector@_N$01@@$0A@@@A" = external global %"class.RWTexture2DMS<vector<bool, 2>, 0>", align 4
@"\01?LTex2dMs@@3V?$RWTexture2DMS@V?$vector@_K$01@@$0A@@@A" = external global %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>", align 8
@"\01?DTex2dMs@@3V?$RWTexture2DMS@N$0A@@@A" = external global %"class.RWTexture2DMS<double, 0>", align 8

; Function Attrs: nounwind
; CHECK-LABEL: define void @main(i32 %ix1, <2 x i32> %ix2, <3 x i32> %ix3)
define void @main(i32 %ix1, <2 x i32> %ix2, <3 x i32> %ix3) #0 {
bb:
  ; CHECK: [[ix3_0:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 0, i32 undef)
  ; CHECK: [[ix3:%.*]] = insertelement <3 x i32> undef, i32 [[ix3_0]], i64 0
  ; CHECK: [[ix3_1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 1, i32 undef)
  ; CHECK: [[vec3:%.*]] = insertelement <3 x i32> [[ix3]], i32 [[ix3_1]], i64 1
  ; CHECK: [[ix3_2:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 2, i32 undef)
  ; CHECK: [[ix3:%.*]] = insertelement <3 x i32> [[vec3]], i32 [[ix3_2]], i64 2
  ; CHECK: [[ix2_0:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  ; CHECK: [[vec2:%.*]] = insertelement <2 x i32> undef, i32 [[ix2_0]], i64 0
  ; CHECK: [[ix2_1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 1, i32 undef)
  ; CHECK: [[ix2:%.*]] = insertelement <2 x i32> [[vec2]], i32 [[ix2_1]], i64 1
  ; CHECK: [[ix1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<float, 3> >"(i32 160, %"class.RWBuffer<vector<float, 3> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 777 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle [[anhdl]], i32 [[ix1]], i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[ping:%.*]] = insertelement <3 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <3 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[vec:%.*]] = insertelement <3 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 1
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<float, 3> >"(i32 160, %"class.RWBuffer<vector<float, 3> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 777 })
  ; CHECK: [[val3:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <3 x float> [[vec]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <3 x float> [[vec]], i64 2
  ; CHECK: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, float [[val0]], float [[val1]], float [[val2]], float [[val3]], i8 15)
  %tmp = load %"class.RWBuffer<vector<float, 3> >", %"class.RWBuffer<vector<float, 3> >"* @"\01?FTyBuf@@3V?$RWBuffer@V?$vector@M$02@@@@A"
  %tmp1 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 3> >\22)"(i32 0, %"class.RWBuffer<vector<float, 3> >" %tmp)
  %tmp2 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 3> >\22)"(i32 14, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4106, i32 777 }, %"class.RWBuffer<vector<float, 3> >" zeroinitializer)
  %tmp3 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp2, i32 %ix1)
  %tmp4 = load <3 x float>, <3 x float>* %tmp3
  %tmp5 = add i32 %ix1, 1
  %tmp6 = load %"class.RWBuffer<vector<float, 3> >", %"class.RWBuffer<vector<float, 3> >"* @"\01?FTyBuf@@3V?$RWBuffer@V?$vector@M$02@@@@A"
  %tmp7 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 3> >\22)"(i32 0, %"class.RWBuffer<vector<float, 3> >" %tmp6)
  %tmp8 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 3> >\22)"(i32 14, %dx.types.Handle %tmp7, %dx.types.ResourceProperties { i32 4106, i32 777 }, %"class.RWBuffer<vector<float, 3> >" zeroinitializer)
  %tmp9 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp8, i32 %tmp5)
  store <3 x float> %tmp4, <3 x float>* %tmp9

  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 2
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<bool, 2> >"(i32 160, %"class.RWBuffer<vector<bool, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 517 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x i32> undef, i32 [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <2 x i32> [[ping]], i32 [[val1]], i64 1
  ; CHECK: [[bvec:%.*]] = icmp ne <2 x i32> [[pong]], zeroinitializer
  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 3
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<bool, 2> >"(i32 160, %"class.RWBuffer<vector<bool, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 517 })
  ; CHECK: [[vec:%.*]] = zext <2 x i1> [[bvec]] to <2 x i32>
  ; CHECK: [[val3:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i32> [[vec]], i64 1
  ; CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, i32 [[val0]], i32 [[val1]], i32 [[val3]], i32 [[val3]], i8 15)
  %tmp10 = add i32 %ix1, 2
  %tmp11 = load %"class.RWBuffer<vector<bool, 2> >", %"class.RWBuffer<vector<bool, 2> >"* @"\01?BTyBuf@@3V?$RWBuffer@V?$vector@_N$01@@@@A"
  %tmp12 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 0, %"class.RWBuffer<vector<bool, 2> >" %tmp11)
  %tmp13 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %tmp12, %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<bool, 2> >" zeroinitializer)
  %tmp14 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp13, i32 %tmp10)
  %tmp15 = load <2 x i32>, <2 x i32>* %tmp14
  %tmp16 = icmp ne <2 x i32> %tmp15, zeroinitializer
  %tmp17 = add i32 %ix1, 3
  %tmp18 = load %"class.RWBuffer<vector<bool, 2> >", %"class.RWBuffer<vector<bool, 2> >"* @"\01?BTyBuf@@3V?$RWBuffer@V?$vector@_N$01@@@@A"
  %tmp19 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 0, %"class.RWBuffer<vector<bool, 2> >" %tmp18)
  %tmp20 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %tmp19, %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<bool, 2> >" zeroinitializer)
  %tmp21 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp20, i32 %tmp17)
  %tmp22 = zext <2 x i1> %tmp16 to <2 x i32>
  store <2 x i32> %tmp22, <2 x i32>* %tmp21

  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 4
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<unsigned long long, 2> >"(i32 160, %"class.RWBuffer<vector<unsigned long long, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 517 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 3
  ; CHECK: [[loval:%.*]] = zext i32 [[val0]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val1]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val0:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[loval:%.*]] = zext i32 [[val2]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val3]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val1:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[ping:%.*]] = insertelement <2 x i64> undef, i64 [[val0]], i64 0
  ; CHECK: [[vec:%.*]] = insertelement <2 x i64> [[ping]], i64 [[val1]], i64 1
  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 5
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<unsigned long long, 2> >"(i32 160, %"class.RWBuffer<vector<unsigned long long, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 517 })
  ; CHECK: [[val3:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i64> [[vec]], i64 1
  ; CHECK: [[loval0:%.*]] = trunc i64 [[val0]] to i32
  ; CHECK: [[msk0:%.*]] = lshr i64 [[val0]], 32
  ; CHECK: [[hival0:%.*]] = trunc i64 [[msk0]] to i32
  ; CHECK: [[loval1:%.*]] = trunc i64 [[val1]] to i32
  ; CHECK: [[msk1:%.*]] = lshr i64 [[val1]], 32
  ; CHECK: [[hival1:%.*]] = trunc i64 [[msk1]] to i32
  ; CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, i32 [[loval0]], i32 [[hival0]], i32 [[loval1]], i32 [[hival1]], i8 15)
  %tmp23 = add i32 %ix1, 4
  %tmp24 = load %"class.RWBuffer<vector<unsigned long long, 2> >", %"class.RWBuffer<vector<unsigned long long, 2> >"* @"\01?LTyBuf@@3V?$RWBuffer@V?$vector@_K$01@@@@A"
  %tmp25 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWBuffer<vector<unsigned long long, 2> >" %tmp24)
  %tmp26 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle %tmp25, %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<unsigned long long, 2> >" zeroinitializer)
  %tmp27 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp26, i32 %tmp23)
  %tmp28 = load <2 x i64>, <2 x i64>* %tmp27
  %tmp29 = add i32 %ix1, 5
  %tmp30 = load %"class.RWBuffer<vector<unsigned long long, 2> >", %"class.RWBuffer<vector<unsigned long long, 2> >"* @"\01?LTyBuf@@3V?$RWBuffer@V?$vector@_K$01@@@@A"
  %tmp31 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWBuffer<vector<unsigned long long, 2> >" %tmp30)
  %tmp32 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle %tmp31, %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<unsigned long long, 2> >" zeroinitializer)
  %tmp33 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp32, i32 %tmp29)
  store <2 x i64> %tmp28, <2 x i64>* %tmp33


  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 6
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<double>"(i32 160, %"class.RWBuffer<double>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 261 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[dval:%.*]] = call double @dx.op.makeDouble.f64(i32 101, i32 [[val0]], i32 [[val1]])
  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 7
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<double>"(i32 160, %"class.RWBuffer<double>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 261 })
  ; CHECK: [[dvec:%.*]] = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double [[dval]])
  ; CHECK: [[lodbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 0
  ; CHECK: [[hidbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 1
  ; CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, i32 [[lodbl]], i32 [[hidbl]], i32 [[lodbl]], i32 [[hidbl]], i8 15)
  %tmp34 = add i32 %ix1, 6
  %tmp35 = load %"class.RWBuffer<double>", %"class.RWBuffer<double>"* @"\01?DTyBuf@@3V?$RWBuffer@N@@A"
  %tmp36 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<double>\22)"(i32 0, %"class.RWBuffer<double>" %tmp35)
  %tmp37 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<double>\22)"(i32 14, %dx.types.Handle %tmp36, %dx.types.ResourceProperties { i32 4106, i32 261 }, %"class.RWBuffer<double>" zeroinitializer)
  %tmp38 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp37, i32 %tmp34)
  %tmp39 = load double, double* %tmp38
  %tmp40 = add i32 %ix1, 7
  %tmp41 = load %"class.RWBuffer<double>", %"class.RWBuffer<double>"* @"\01?DTyBuf@@3V?$RWBuffer@N@@A"
  %tmp42 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<double>\22)"(i32 0, %"class.RWBuffer<double>" %tmp41)
  %tmp43 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<double>\22)"(i32 14, %dx.types.Handle %tmp42, %dx.types.ResourceProperties { i32 4106, i32 261 }, %"class.RWBuffer<double>" zeroinitializer)
  %tmp44 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp43, i32 %tmp40)
  store double %tmp39, double* %tmp44

  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 8
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture1D<vector<float, 3> >"(i32 160, %"class.RWTexture1D<vector<float, 3> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 777 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix]], i32 undef, i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[ping:%.*]] = insertelement <3 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <3 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[vec:%.*]] = insertelement <3 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 9
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture1D<vector<float, 3> >"(i32 160, %"class.RWTexture1D<vector<float, 3> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 777 })
  ; CHECK: [[val3:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <3 x float> [[vec]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <3 x float> [[vec]], i64 2
  ; CHECK: call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, i32 undef, float [[val0]], float [[val1]], float [[val2]], float [[val3]], i8 15)
  %tmp45 = add i32 %ix1, 8
  %tmp46 = load %"class.RWTexture1D<vector<float, 3> >", %"class.RWTexture1D<vector<float, 3> >"* @"\01?FTex1d@@3V?$RWTexture1D@V?$vector@M$02@@@@A"
  %tmp47 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture1D<vector<float, 3> >" %tmp46)
  %tmp48 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle %tmp47, %dx.types.ResourceProperties { i32 4097, i32 777 }, %"class.RWTexture1D<vector<float, 3> >" zeroinitializer)
  %tmp49 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp48, i32 %tmp45)
  %tmp50 = load <3 x float>, <3 x float>* %tmp49
  %tmp51 = add i32 %ix1, 9
  %tmp52 = load %"class.RWTexture1D<vector<float, 3> >", %"class.RWTexture1D<vector<float, 3> >"* @"\01?FTex1d@@3V?$RWTexture1D@V?$vector@M$02@@@@A"
  %tmp53 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture1D<vector<float, 3> >" %tmp52)
  %tmp54 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle %tmp53, %dx.types.ResourceProperties { i32 4097, i32 777 }, %"class.RWTexture1D<vector<float, 3> >" zeroinitializer)
  %tmp55 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp54, i32 %tmp51)
  store <3 x float> %tmp50, <3 x float>* %tmp55

  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 10
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture1D<vector<bool, 2> >"(i32 160, %"class.RWTexture1D<vector<bool, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 517 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix]], i32 undef, i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x i32> undef, i32 [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <2 x i32> [[ping]], i32 [[val1]], i64 1
  ; CHECK: [[bvec:%.*]] = icmp ne <2 x i32> [[pong]], zeroinitializer
  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 11
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture1D<vector<bool, 2> >"(i32 160, %"class.RWTexture1D<vector<bool, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 517 })
  ; CHECK: [[vec:%.*]] = zext <2 x i1> [[bvec]] to <2 x i32>
  ; CHECK: [[val3:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i32> [[vec]], i64 1
  ; CHECK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, i32 undef, i32 [[val0]], i32 [[val1]], i32 [[val3]], i32 [[val3]], i8 15)
  %tmp56 = add i32 %ix1, 10
  %tmp57 = load %"class.RWTexture1D<vector<bool, 2> >", %"class.RWTexture1D<vector<bool, 2> >"* @"\01?BTex1d@@3V?$RWTexture1D@V?$vector@_N$01@@@@A"
  %tmp58 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture1D<vector<bool, 2> >" %tmp57)
  %tmp59 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %tmp58, %dx.types.ResourceProperties { i32 4097, i32 517 }, %"class.RWTexture1D<vector<bool, 2> >" zeroinitializer)
  %tmp60 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp59, i32 %tmp56)
  %tmp61 = load <2 x i32>, <2 x i32>* %tmp60
  %tmp62 = icmp ne <2 x i32> %tmp61, zeroinitializer
  %tmp63 = add i32 %ix1, 11
  %tmp64 = load %"class.RWTexture1D<vector<bool, 2> >", %"class.RWTexture1D<vector<bool, 2> >"* @"\01?BTex1d@@3V?$RWTexture1D@V?$vector@_N$01@@@@A"
  %tmp65 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture1D<vector<bool, 2> >" %tmp64)
  %tmp66 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %tmp65, %dx.types.ResourceProperties { i32 4097, i32 517 }, %"class.RWTexture1D<vector<bool, 2> >" zeroinitializer)
  %tmp67 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp66, i32 %tmp63)
  %tmp68 = zext <2 x i1> %tmp62 to <2 x i32>
  store <2 x i32> %tmp68, <2 x i32>* %tmp67

  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 12
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture1D<vector<unsigned long long, 2> >"(i32 160, %"class.RWTexture1D<vector<unsigned long long, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 517 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix]], i32 undef, i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 3
  ; CHECK: [[loval:%.*]] = zext i32 [[val0]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val1]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val0:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[loval:%.*]] = zext i32 [[val2]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val3]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val1:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[ping:%.*]] = insertelement <2 x i64> undef, i64 [[val0]], i64 0
  ; CHECK: [[vec:%.*]] = insertelement <2 x i64> [[ping]], i64 [[val1]], i64 1
  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 13
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture1D<vector<unsigned long long, 2> >"(i32 160, %"class.RWTexture1D<vector<unsigned long long, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 517 })
  ; CHECK: [[val3:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i64> [[vec]], i64 1
  ; CHECK: [[loval0:%.*]] = trunc i64 [[val0]] to i32
  ; CHECK: [[msk0:%.*]] = lshr i64 [[val0]], 32
  ; CHECK: [[hival0:%.*]] = trunc i64 [[msk0]] to i32
  ; CHECK: [[loval1:%.*]] = trunc i64 [[val1]] to i32
  ; CHECK: [[msk1:%.*]] = lshr i64 [[val1]], 32
  ; CHECK: [[hival1:%.*]] = trunc i64 [[msk1]] to i32
  ; CHECK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, i32 undef, i32 [[loval0]], i32 [[hival0]], i32 [[loval1]], i32 [[hival1]], i8 15)
  %tmp69 = add i32 %ix1, 12
  %tmp70 = load %"class.RWTexture1D<vector<unsigned long long, 2> >", %"class.RWTexture1D<vector<unsigned long long, 2> >"* @"\01?LTex1d@@3V?$RWTexture1D@V?$vector@_K$01@@@@A"
  %tmp71 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture1D<vector<unsigned long long, 2> >" %tmp70)
  %tmp72 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle %tmp71, %dx.types.ResourceProperties { i32 4097, i32 517 }, %"class.RWTexture1D<vector<unsigned long long, 2> >" zeroinitializer)
  %tmp73 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp72, i32 %tmp69)
  %tmp74 = load <2 x i64>, <2 x i64>* %tmp73
  %tmp75 = add i32 %ix1, 13
  %tmp76 = load %"class.RWTexture1D<vector<unsigned long long, 2> >", %"class.RWTexture1D<vector<unsigned long long, 2> >"* @"\01?LTex1d@@3V?$RWTexture1D@V?$vector@_K$01@@@@A"
  %tmp77 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture1D<vector<unsigned long long, 2> >" %tmp76)
  %tmp78 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle %tmp77, %dx.types.ResourceProperties { i32 4097, i32 517 }, %"class.RWTexture1D<vector<unsigned long long, 2> >" zeroinitializer)
  %tmp79 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp78, i32 %tmp75)
  store <2 x i64> %tmp74, <2 x i64>* %tmp79


  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 14
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture1D<double>"(i32 160, %"class.RWTexture1D<double>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 261 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix]], i32 undef, i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[dval:%.*]] = call double @dx.op.makeDouble.f64(i32 101, i32 [[val0]], i32 [[val1]])
  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 15
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture1D<double>"(i32 160, %"class.RWTexture1D<double>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 261 })
  ; CHECK: [[dvec:%.*]] = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double [[dval]])
  ; CHECK: [[lodbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 0
  ; CHECK: [[hidbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 1
  ; CHECK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, i32 undef, i32 [[lodbl]], i32 [[hidbl]], i32 [[lodbl]], i32 [[hidbl]], i8 15)
  %tmp80 = add i32 %ix1, 14
  %tmp81 = load %"class.RWTexture1D<double>", %"class.RWTexture1D<double>"* @"\01?DTex1d@@3V?$RWTexture1D@N@@A"
  %tmp82 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<double>\22)"(i32 0, %"class.RWTexture1D<double>" %tmp81)
  %tmp83 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<double>\22)"(i32 14, %dx.types.Handle %tmp82, %dx.types.ResourceProperties { i32 4097, i32 261 }, %"class.RWTexture1D<double>" zeroinitializer)
  %tmp84 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp83, i32 %tmp80)
  %tmp85 = load double, double* %tmp84
  %tmp86 = add i32 %ix1, 15
  %tmp87 = load %"class.RWTexture1D<double>", %"class.RWTexture1D<double>"* @"\01?DTex1d@@3V?$RWTexture1D@N@@A"
  %tmp88 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<double>\22)"(i32 0, %"class.RWTexture1D<double>" %tmp87)
  %tmp89 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<double>\22)"(i32 14, %dx.types.Handle %tmp88, %dx.types.ResourceProperties { i32 4097, i32 261 }, %"class.RWTexture1D<double>" zeroinitializer)
  %tmp90 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp89, i32 %tmp86)
  store double %tmp85, double* %tmp90

  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 16, i32 16>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<vector<float, 3> >"(i32 160, %"class.RWTexture2D<vector<float, 3> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 777 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[ping:%.*]] = insertelement <3 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <3 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[vec:%.*]] = insertelement <3 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 17, i32 17>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<vector<float, 3> >"(i32 160, %"class.RWTexture2D<vector<float, 3> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 777 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[val3:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <3 x float> [[vec]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <3 x float> [[vec]], i64 2
  ; CHECK: call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, float [[val0]], float [[val1]], float [[val2]], float [[val3]], i8 15)
  %tmp91 = add <2 x i32> %ix2, <i32 16, i32 16>
  %tmp92 = load %"class.RWTexture2D<vector<float, 3> >", %"class.RWTexture2D<vector<float, 3> >"* @"\01?FTex2d@@3V?$RWTexture2D@V?$vector@M$02@@@@A"
  %tmp93 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture2D<vector<float, 3> >" %tmp92)
  %tmp94 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle %tmp93, %dx.types.ResourceProperties { i32 4098, i32 777 }, %"class.RWTexture2D<vector<float, 3> >" zeroinitializer)
  %tmp95 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp94, <2 x i32> %tmp91)
  %tmp96 = load <3 x float>, <3 x float>* %tmp95
  %tmp97 = add <2 x i32> %ix2, <i32 17, i32 17>
  %tmp98 = load %"class.RWTexture2D<vector<float, 3> >", %"class.RWTexture2D<vector<float, 3> >"* @"\01?FTex2d@@3V?$RWTexture2D@V?$vector@M$02@@@@A"
  %tmp99 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture2D<vector<float, 3> >" %tmp98)
  %tmp100 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle %tmp99, %dx.types.ResourceProperties { i32 4098, i32 777 }, %"class.RWTexture2D<vector<float, 3> >" zeroinitializer)
  %tmp101 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp100, <2 x i32> %tmp97)
  store <3 x float> %tmp96, <3 x float>* %tmp101

  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 18, i32 18>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<vector<bool, 2> >"(i32 160, %"class.RWTexture2D<vector<bool, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 517 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x i32> undef, i32 [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <2 x i32> [[ping]], i32 [[val1]], i64 1
  ; CHECK: [[bvec:%.*]] = icmp ne <2 x i32> [[pong]], zeroinitializer
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 19, i32 19>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<vector<bool, 2> >"(i32 160, %"class.RWTexture2D<vector<bool, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 517 })
  ; CHECK: [[vec:%.*]] = zext <2 x i1> [[bvec]] to <2 x i32>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[val3:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i32> [[vec]], i64 1
  ; CHECK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 [[val0]], i32 [[val1]], i32 [[val3]], i32 [[val3]], i8 15)
  %tmp102 = add <2 x i32> %ix2, <i32 18, i32 18>
  %tmp103 = load %"class.RWTexture2D<vector<bool, 2> >", %"class.RWTexture2D<vector<bool, 2> >"* @"\01?BTex2d@@3V?$RWTexture2D@V?$vector@_N$01@@@@A"
  %tmp104 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture2D<vector<bool, 2> >" %tmp103)
  %tmp105 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %tmp104, %dx.types.ResourceProperties { i32 4098, i32 517 }, %"class.RWTexture2D<vector<bool, 2> >" zeroinitializer)
  %tmp106 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp105, <2 x i32> %tmp102)
  %tmp107 = load <2 x i32>, <2 x i32>* %tmp106
  %tmp108 = icmp ne <2 x i32> %tmp107, zeroinitializer
  %tmp109 = add <2 x i32> %ix2, <i32 19, i32 19>
  %tmp110 = load %"class.RWTexture2D<vector<bool, 2> >", %"class.RWTexture2D<vector<bool, 2> >"* @"\01?BTex2d@@3V?$RWTexture2D@V?$vector@_N$01@@@@A"
  %tmp111 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture2D<vector<bool, 2> >" %tmp110)
  %tmp112 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %tmp111, %dx.types.ResourceProperties { i32 4098, i32 517 }, %"class.RWTexture2D<vector<bool, 2> >" zeroinitializer)
  %tmp113 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp112, <2 x i32> %tmp109)
  %tmp114 = zext <2 x i1> %tmp108 to <2 x i32>
  store <2 x i32> %tmp114, <2 x i32>* %tmp113

  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 20, i32 20>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<vector<unsigned long long, 2> >"(i32 160, %"class.RWTexture2D<vector<unsigned long long, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 517 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 3
  ; CHECK: [[loval:%.*]] = zext i32 [[val0]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val1]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val0:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[loval:%.*]] = zext i32 [[val2]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val3]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val1:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[ping:%.*]] = insertelement <2 x i64> undef, i64 [[val0]], i64 0
  ; CHECK: [[vec:%.*]] = insertelement <2 x i64> [[ping]], i64 [[val1]], i64 1
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 21, i32 21>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<vector<unsigned long long, 2> >"(i32 160, %"class.RWTexture2D<vector<unsigned long long, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 517 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[val3:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i64> [[vec]], i64 1
  ; CHECK: [[loval0:%.*]] = trunc i64 [[val0]] to i32
  ; CHECK: [[msk0:%.*]] = lshr i64 [[val0]], 32
  ; CHECK: [[hival0:%.*]] = trunc i64 [[msk0]] to i32
  ; CHECK: [[loval1:%.*]] = trunc i64 [[val1]] to i32
  ; CHECK: [[msk1:%.*]] = lshr i64 [[val1]], 32
  ; CHECK: [[hival1:%.*]] = trunc i64 [[msk1]] to i32
  ; CHECK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 [[loval0]], i32 [[hival0]], i32 [[loval1]], i32 [[hival1]], i8 15)
  %tmp115 = add <2 x i32> %ix2, <i32 20, i32 20>
  %tmp116 = load %"class.RWTexture2D<vector<unsigned long long, 2> >", %"class.RWTexture2D<vector<unsigned long long, 2> >"* @"\01?LTex2d@@3V?$RWTexture2D@V?$vector@_K$01@@@@A"
  %tmp117 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture2D<vector<unsigned long long, 2> >" %tmp116)
  %tmp118 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle %tmp117, %dx.types.ResourceProperties { i32 4098, i32 517 }, %"class.RWTexture2D<vector<unsigned long long, 2> >" zeroinitializer)
  %tmp119 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp118, <2 x i32> %tmp115)
  %tmp120 = load <2 x i64>, <2 x i64>* %tmp119
  %tmp121 = add <2 x i32> %ix2, <i32 21, i32 21>
  %tmp122 = load %"class.RWTexture2D<vector<unsigned long long, 2> >", %"class.RWTexture2D<vector<unsigned long long, 2> >"* @"\01?LTex2d@@3V?$RWTexture2D@V?$vector@_K$01@@@@A"
  %tmp123 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture2D<vector<unsigned long long, 2> >" %tmp122)
  %tmp124 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle %tmp123, %dx.types.ResourceProperties { i32 4098, i32 517 }, %"class.RWTexture2D<vector<unsigned long long, 2> >" zeroinitializer)
  %tmp125 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp124, <2 x i32> %tmp121)
  store <2 x i64> %tmp120, <2 x i64>* %tmp125

  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 22, i32 22>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<double>"(i32 160, %"class.RWTexture2D<double>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 261 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[dval:%.*]] = call double @dx.op.makeDouble.f64(i32 101, i32 [[val0]], i32 [[val1]])
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 23, i32 23>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<double>"(i32 160, %"class.RWTexture2D<double>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 261 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[dvec:%.*]] = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double [[dval]])
  ; CHECK: [[lodbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 0
  ; CHECK: [[hidbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 1
  ; CHECK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 [[lodbl]], i32 [[hidbl]], i32 [[lodbl]], i32 [[hidbl]], i8 15)
  %tmp126 = add <2 x i32> %ix2, <i32 22, i32 22>
  %tmp127 = load %"class.RWTexture2D<double>", %"class.RWTexture2D<double>"* @"\01?DTex2d@@3V?$RWTexture2D@N@@A"
  %tmp128 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<double>\22)"(i32 0, %"class.RWTexture2D<double>" %tmp127)
  %tmp129 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<double>\22)"(i32 14, %dx.types.Handle %tmp128, %dx.types.ResourceProperties { i32 4098, i32 261 }, %"class.RWTexture2D<double>" zeroinitializer)
  %tmp130 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp129, <2 x i32> %tmp126)
  %tmp131 = load double, double* %tmp130
  %tmp132 = add <2 x i32> %ix2, <i32 23, i32 23>
  %tmp133 = load %"class.RWTexture2D<double>", %"class.RWTexture2D<double>"* @"\01?DTex2d@@3V?$RWTexture2D@N@@A"
  %tmp134 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<double>\22)"(i32 0, %"class.RWTexture2D<double>" %tmp133)
  %tmp135 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<double>\22)"(i32 14, %dx.types.Handle %tmp134, %dx.types.ResourceProperties { i32 4098, i32 261 }, %"class.RWTexture2D<double>" zeroinitializer)
  %tmp136 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp135, <2 x i32> %tmp132)
  store double %tmp131, double* %tmp136

  ; CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 24, i32 24, i32 24>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture3D<vector<float, 3> >"(i32 160, %"class.RWTexture3D<vector<float, 3> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 777 })
  ; CHECK: [[ix3_0:%.*]] = extractelement <3 x i32> [[ix]], i64 0
  ; CHECK: [[ix3_1:%.*]] = extractelement <3 x i32> [[ix]], i64 1
  ; CHECK: [[ix3_2:%.*]] = extractelement <3 x i32> [[ix]], i64 2
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix3_0]], i32 [[ix3_1]], i32 [[ix3_2]], i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[ping:%.*]] = insertelement <3 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <3 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[vec:%.*]] = insertelement <3 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 25, i32 25, i32 25>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture3D<vector<float, 3> >"(i32 160, %"class.RWTexture3D<vector<float, 3> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 777 })
  ; CHECK: [[ix3_0:%.*]] = extractelement <3 x i32> [[ix]], i64 0
  ; CHECK: [[ix3_1:%.*]] = extractelement <3 x i32> [[ix]], i64 1
  ; CHECK: [[ix3_2:%.*]] = extractelement <3 x i32> [[ix]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <3 x float> [[vec]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <3 x float> [[vec]], i64 2
  ; CHECK: call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix3_0]], i32 [[ix3_1]], i32 [[ix3_2]], float [[val0]], float [[val1]], float [[val2]], float [[val3]], i8 15)
  %tmp137 = add <3 x i32> %ix3, <i32 24, i32 24, i32 24>
  %tmp138 = load %"class.RWTexture3D<vector<float, 3> >", %"class.RWTexture3D<vector<float, 3> >"* @"\01?FTex3d@@3V?$RWTexture3D@V?$vector@M$02@@@@A"
  %tmp139 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture3D<vector<float, 3> >" %tmp138)
  %tmp140 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle %tmp139, %dx.types.ResourceProperties { i32 4100, i32 777 }, %"class.RWTexture3D<vector<float, 3> >" zeroinitializer)
  %tmp141 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %tmp140, <3 x i32> %tmp137)
  %tmp142 = load <3 x float>, <3 x float>* %tmp141
  %tmp143 = add <3 x i32> %ix3, <i32 25, i32 25, i32 25>
  %tmp144 = load %"class.RWTexture3D<vector<float, 3> >", %"class.RWTexture3D<vector<float, 3> >"* @"\01?FTex3d@@3V?$RWTexture3D@V?$vector@M$02@@@@A"
  %tmp145 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture3D<vector<float, 3> >" %tmp144)
  %tmp146 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle %tmp145, %dx.types.ResourceProperties { i32 4100, i32 777 }, %"class.RWTexture3D<vector<float, 3> >" zeroinitializer)
  %tmp147 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %tmp146, <3 x i32> %tmp143)
  store <3 x float> %tmp142, <3 x float>* %tmp147

  ; CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 26, i32 26, i32 26>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture3D<vector<bool, 2> >"(i32 160, %"class.RWTexture3D<vector<bool, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 517 })
  ; CHECK: [[ix3_0:%.*]] = extractelement <3 x i32> [[ix]], i64 0
  ; CHECK: [[ix3_1:%.*]] = extractelement <3 x i32> [[ix]], i64 1
  ; CHECK: [[ix3_2:%.*]] = extractelement <3 x i32> [[ix]], i64 2
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix3_0]], i32 [[ix3_1]], i32 [[ix3_2]], i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x i32> undef, i32 [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <2 x i32> [[ping]], i32 [[val1]], i64 1
  ; CHECK: [[bvec:%.*]] = icmp ne <2 x i32> [[pong]], zeroinitializer
  ; CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 27, i32 27, i32 27>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture3D<vector<bool, 2> >"(i32 160, %"class.RWTexture3D<vector<bool, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 517 })
  ; CHECK: [[vec:%.*]] = zext <2 x i1> [[bvec]] to <2 x i32>
  ; CHECK: [[ix3_0:%.*]] = extractelement <3 x i32> [[ix]], i64 0
  ; CHECK: [[ix3_1:%.*]] = extractelement <3 x i32> [[ix]], i64 1
  ; CHECK: [[ix3_2:%.*]] = extractelement <3 x i32> [[ix]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i32> [[vec]], i64 1
  ; CHECK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix3_0]], i32 [[ix3_1]], i32 [[ix3_2]], i32 [[val0]], i32 [[val1]], i32 [[val3]], i32 [[val3]], i8 15)
  %tmp148 = add <3 x i32> %ix3, <i32 26, i32 26, i32 26>
  %tmp149 = load %"class.RWTexture3D<vector<bool, 2> >", %"class.RWTexture3D<vector<bool, 2> >"* @"\01?BTex3d@@3V?$RWTexture3D@V?$vector@_N$01@@@@A"
  %tmp150 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture3D<vector<bool, 2> >" %tmp149)
  %tmp151 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %tmp150, %dx.types.ResourceProperties { i32 4100, i32 517 }, %"class.RWTexture3D<vector<bool, 2> >" zeroinitializer)
  %tmp152 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %tmp151, <3 x i32> %tmp148)
  %tmp153 = load <2 x i32>, <2 x i32>* %tmp152
  %tmp154 = icmp ne <2 x i32> %tmp153, zeroinitializer
  %tmp155 = add <3 x i32> %ix3, <i32 27, i32 27, i32 27>
  %tmp156 = load %"class.RWTexture3D<vector<bool, 2> >", %"class.RWTexture3D<vector<bool, 2> >"* @"\01?BTex3d@@3V?$RWTexture3D@V?$vector@_N$01@@@@A"
  %tmp157 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture3D<vector<bool, 2> >" %tmp156)
  %tmp158 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %tmp157, %dx.types.ResourceProperties { i32 4100, i32 517 }, %"class.RWTexture3D<vector<bool, 2> >" zeroinitializer)
  %tmp159 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %tmp158, <3 x i32> %tmp155)
  %tmp160 = zext <2 x i1> %tmp154 to <2 x i32>
  store <2 x i32> %tmp160, <2 x i32>* %tmp159

  ; CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 28, i32 28, i32 28>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture3D<vector<unsigned long long, 2> >"(i32 160, %"class.RWTexture3D<vector<unsigned long long, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 517 })
  ; CHECK: [[ix3_0:%.*]] = extractelement <3 x i32> [[ix]], i64 0
  ; CHECK: [[ix3_1:%.*]] = extractelement <3 x i32> [[ix]], i64 1
  ; CHECK: [[ix3_2:%.*]] = extractelement <3 x i32> [[ix]], i64 2
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix3_0]], i32 [[ix3_1]], i32 [[ix3_2]], i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 3
  ; CHECK: [[loval:%.*]] = zext i32 [[val0]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val1]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val0:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[loval:%.*]] = zext i32 [[val2]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val3]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val1:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[ping:%.*]] = insertelement <2 x i64> undef, i64 [[val0]], i64 0
  ; CHECK: [[vec:%.*]] = insertelement <2 x i64> [[ping]], i64 [[val1]], i64 1
  ; CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 29, i32 29, i32 29>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture3D<vector<unsigned long long, 2> >"(i32 160, %"class.RWTexture3D<vector<unsigned long long, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 517 })
  ; CHECK: [[ix3_0:%.*]] = extractelement <3 x i32> [[ix]], i64 0
  ; CHECK: [[ix3_1:%.*]] = extractelement <3 x i32> [[ix]], i64 1
  ; CHECK: [[ix3_2:%.*]] = extractelement <3 x i32> [[ix]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i64> [[vec]], i64 1
  ; CHECK: [[loval0:%.*]] = trunc i64 [[val0]] to i32
  ; CHECK: [[msk0:%.*]] = lshr i64 [[val0]], 32
  ; CHECK: [[hival0:%.*]] = trunc i64 [[msk0]] to i32
  ; CHECK: [[loval1:%.*]] = trunc i64 [[val1]] to i32
  ; CHECK: [[msk1:%.*]] = lshr i64 [[val1]], 32
  ; CHECK: [[hival1:%.*]] = trunc i64 [[msk1]] to i32
  ; CHECK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix3_0]], i32 [[ix3_1]], i32 [[ix3_2]], i32 [[loval0]], i32 [[hival0]], i32 [[loval1]], i32 [[hival1]], i8 15)
  %tmp161 = add <3 x i32> %ix3, <i32 28, i32 28, i32 28>
  %tmp162 = load %"class.RWTexture3D<vector<unsigned long long, 2> >", %"class.RWTexture3D<vector<unsigned long long, 2> >"* @"\01?LTex3d@@3V?$RWTexture3D@V?$vector@_K$01@@@@A"
  %tmp163 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture3D<vector<unsigned long long, 2> >" %tmp162)
  %tmp164 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle %tmp163, %dx.types.ResourceProperties { i32 4100, i32 517 }, %"class.RWTexture3D<vector<unsigned long long, 2> >" zeroinitializer)
  %tmp165 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %tmp164, <3 x i32> %tmp161)
  %tmp166 = load <2 x i64>, <2 x i64>* %tmp165
  %tmp167 = add <3 x i32> %ix3, <i32 29, i32 29, i32 29>
  %tmp168 = load %"class.RWTexture3D<vector<unsigned long long, 2> >", %"class.RWTexture3D<vector<unsigned long long, 2> >"* @"\01?LTex3d@@3V?$RWTexture3D@V?$vector@_K$01@@@@A"
  %tmp169 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture3D<vector<unsigned long long, 2> >" %tmp168)
  %tmp170 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle %tmp169, %dx.types.ResourceProperties { i32 4100, i32 517 }, %"class.RWTexture3D<vector<unsigned long long, 2> >" zeroinitializer)
  %tmp171 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %tmp170, <3 x i32> %tmp167)
  store <2 x i64> %tmp166, <2 x i64>* %tmp171

  ; CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 30, i32 30, i32 30>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture3D<double>"(i32 160, %"class.RWTexture3D<double>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 261 })
  ; CHECK: [[ix3_0:%.*]] = extractelement <3 x i32> [[ix]], i64 0
  ; CHECK: [[ix3_1:%.*]] = extractelement <3 x i32> [[ix]], i64 1
  ; CHECK: [[ix3_2:%.*]] = extractelement <3 x i32> [[ix]], i64 2
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 undef, i32 [[ix3_0]], i32 [[ix3_1]], i32 [[ix3_2]], i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[dval:%.*]] = call double @dx.op.makeDouble.f64(i32 101, i32 [[val0]], i32 [[val1]])
  ; CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 31, i32 31, i32 31>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture3D<double>"(i32 160, %"class.RWTexture3D<double>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 261 })
  ; CHECK: [[ix3_0:%.*]] = extractelement <3 x i32> [[ix]], i64 0
  ; CHECK: [[ix3_1:%.*]] = extractelement <3 x i32> [[ix]], i64 1
  ; CHECK: [[ix3_2:%.*]] = extractelement <3 x i32> [[ix]], i64 2
  ; CHECK: [[dvec:%.*]] = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double [[dval]])
  ; CHECK: [[lodbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 0
  ; CHECK: [[hidbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 1
  ; CHECK: call void @dx.op.textureStore.i32(i32 67, %dx.types.Handle [[anhdl]], i32 [[ix3_0]], i32 [[ix3_1]], i32 [[ix3_2]], i32 [[lodbl]], i32 [[hidbl]], i32 [[lodbl]], i32 [[hidbl]], i8 15)
  %tmp172 = add <3 x i32> %ix3, <i32 30, i32 30, i32 30>
  %tmp173 = load %"class.RWTexture3D<double>", %"class.RWTexture3D<double>"* @"\01?DTex3d@@3V?$RWTexture3D@N@@A"
  %tmp174 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<double>\22)"(i32 0, %"class.RWTexture3D<double>" %tmp173)
  %tmp175 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<double>\22)"(i32 14, %dx.types.Handle %tmp174, %dx.types.ResourceProperties { i32 4100, i32 261 }, %"class.RWTexture3D<double>" zeroinitializer)
  %tmp176 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %tmp175, <3 x i32> %tmp172)
  %tmp177 = load double, double* %tmp176
  %tmp178 = add <3 x i32> %ix3, <i32 31, i32 31, i32 31>
  %tmp179 = load %"class.RWTexture3D<double>", %"class.RWTexture3D<double>"* @"\01?DTex3d@@3V?$RWTexture3D@N@@A"
  %tmp180 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<double>\22)"(i32 0, %"class.RWTexture3D<double>" %tmp179)
  %tmp181 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<double>\22)"(i32 14, %dx.types.Handle %tmp180, %dx.types.ResourceProperties { i32 4100, i32 261 }, %"class.RWTexture3D<double>" zeroinitializer)
  %tmp182 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %tmp181, <3 x i32> %tmp178)
  store double %tmp177, double* %tmp182

  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 32, i32 32>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<float, 3>, 0>"(i32 160, %"class.RWTexture2DMS<vector<float, 3>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 777 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[anhdl]], i32 0, i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[ping:%.*]] = insertelement <3 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <3 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[vec:%.*]] = insertelement <3 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 33, i32 33>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<float, 3>, 0>"(i32 160, %"class.RWTexture2DMS<vector<float, 3>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 777 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[val3:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <3 x float> [[vec]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <3 x float> [[vec]], i64 2
  ; CHECK: call void @dx.op.textureStoreSample.f32(i32 225, %dx.types.Handle [[anhdl]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, float [[val0]], float [[val1]], float [[val2]], float [[val3]], i8 15, i32 0)
  %tmp183 = add <2 x i32> %ix2, <i32 32, i32 32>
  %tmp184 = load %"class.RWTexture2DMS<vector<float, 3>, 0>", %"class.RWTexture2DMS<vector<float, 3>, 0>"* @"\01?FTex2dMs@@3V?$RWTexture2DMS@V?$vector@M$02@@$0A@@@A"
  %tmp185 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<float, 3>, 0>" %tmp184)
  %tmp186 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 14, %dx.types.Handle %tmp185, %dx.types.ResourceProperties { i32 4099, i32 777 }, %"class.RWTexture2DMS<vector<float, 3>, 0>" zeroinitializer)
  %tmp187 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp186, <2 x i32> %tmp183)
  %tmp188 = load <3 x float>, <3 x float>* %tmp187
  %tmp189 = add <2 x i32> %ix2, <i32 33, i32 33>
  %tmp190 = load %"class.RWTexture2DMS<vector<float, 3>, 0>", %"class.RWTexture2DMS<vector<float, 3>, 0>"* @"\01?FTex2dMs@@3V?$RWTexture2DMS@V?$vector@M$02@@$0A@@@A"
  %tmp191 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<float, 3>, 0>" %tmp190)
  %tmp192 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 14, %dx.types.Handle %tmp191, %dx.types.ResourceProperties { i32 4099, i32 777 }, %"class.RWTexture2DMS<vector<float, 3>, 0>" zeroinitializer)
  %tmp193 = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp192, <2 x i32> %tmp189)
  store <3 x float> %tmp188, <3 x float>* %tmp193

  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 34, i32 34>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<bool, 2>, 0>"(i32 160, %"class.RWTexture2DMS<vector<bool, 2>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 0, i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x i32> undef, i32 [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <2 x i32> [[ping]], i32 [[val1]], i64 1
  ; CHECK: [[bvec:%.*]] = icmp ne <2 x i32> [[pong]], zeroinitializer
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 35, i32 35>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<bool, 2>, 0>"(i32 160, %"class.RWTexture2DMS<vector<bool, 2>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 })
  ; CHECK: [[vec:%.*]] = zext <2 x i1> [[bvec]] to <2 x i32>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[val3:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i32> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i32> [[vec]], i64 1
  ; CHECK: call void @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle [[anhdl]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 [[val0]], i32 [[val1]], i32 [[val3]], i32 [[val3]], i8 15, i32 0)
  %tmp194 = add <2 x i32> %ix2, <i32 34, i32 34>
  %tmp195 = load %"class.RWTexture2DMS<vector<bool, 2>, 0>", %"class.RWTexture2DMS<vector<bool, 2>, 0>"* @"\01?BTex2dMs@@3V?$RWTexture2DMS@V?$vector@_N$01@@$0A@@@A"
  %tmp196 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<bool, 2>, 0>" %tmp195)
  %tmp197 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle %tmp196, %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<bool, 2>, 0>" zeroinitializer)
  %tmp198 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp197, <2 x i32> %tmp194)
  %tmp199 = load <2 x i32>, <2 x i32>* %tmp198
  %tmp200 = icmp ne <2 x i32> %tmp199, zeroinitializer
  %tmp201 = add <2 x i32> %ix2, <i32 35, i32 35>
  %tmp202 = load %"class.RWTexture2DMS<vector<bool, 2>, 0>", %"class.RWTexture2DMS<vector<bool, 2>, 0>"* @"\01?BTex2dMs@@3V?$RWTexture2DMS@V?$vector@_N$01@@$0A@@@A"
  %tmp203 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<bool, 2>, 0>" %tmp202)
  %tmp204 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle %tmp203, %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<bool, 2>, 0>" zeroinitializer)
  %tmp205 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp204, <2 x i32> %tmp201)
  %tmp206 = zext <2 x i1> %tmp200 to <2 x i32>
  store <2 x i32> %tmp206, <2 x i32>* %tmp205

  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 36, i32 36>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"(i32 160, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 0, i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 3
  ; CHECK: [[loval:%.*]] = zext i32 [[val0]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val1]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val0:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[loval:%.*]] = zext i32 [[val2]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val3]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val1:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[ping:%.*]] = insertelement <2 x i64> undef, i64 [[val0]], i64 0
  ; CHECK: [[vec:%.*]] = insertelement <2 x i64> [[ping]], i64 [[val1]], i64 1
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 37, i32 37>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"(i32 160, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[val3:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i64> [[vec]], i64 1
  ; CHECK: [[loval0:%.*]] = trunc i64 [[val0]] to i32
  ; CHECK: [[msk0:%.*]] = lshr i64 [[val0]], 32
  ; CHECK: [[hival0:%.*]] = trunc i64 [[msk0]] to i32
  ; CHECK: [[loval1:%.*]] = trunc i64 [[val1]] to i32
  ; CHECK: [[msk1:%.*]] = lshr i64 [[val1]], 32
  ; CHECK: [[hival1:%.*]] = trunc i64 [[msk1]] to i32
  ; CHECK: call void @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle [[anhdl]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 [[loval0]], i32 [[hival0]], i32 [[loval1]], i32 [[hival1]], i8 15, i32 0)

  %tmp207 = add <2 x i32> %ix2, <i32 36, i32 36>
  %tmp208 = load %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>", %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"* @"\01?LTex2dMs@@3V?$RWTexture2DMS@V?$vector@_K$01@@$0A@@@A"
  %tmp209 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" %tmp208)
  %tmp210 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 14, %dx.types.Handle %tmp209, %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" zeroinitializer)
  %tmp211 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp210, <2 x i32> %tmp207)
  %tmp212 = load <2 x i64>, <2 x i64>* %tmp211
  %tmp213 = add <2 x i32> %ix2, <i32 37, i32 37>
  %tmp214 = load %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>", %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"* @"\01?LTex2dMs@@3V?$RWTexture2DMS@V?$vector@_K$01@@$0A@@@A"
  %tmp215 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" %tmp214)
  %tmp216 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 14, %dx.types.Handle %tmp215, %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" zeroinitializer)
  %tmp217 = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp216, <2 x i32> %tmp213)
  store <2 x i64> %tmp212, <2 x i64>* %tmp217
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 38, i32 38>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<double, 0>"(i32 160, %"class.RWTexture2DMS<double, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 261 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 0, i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[dval:%.*]] = call double @dx.op.makeDouble.f64(i32 101, i32 [[val0]], i32 [[val1]])
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 39, i32 39>

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<double, 0>"(i32 160, %"class.RWTexture2DMS<double, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 261 })
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[dvec:%.*]] = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double [[dval]])
  ; CHECK: [[lodbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 0
  ; CHECK: [[hidbl:%.*]] = extractvalue %dx.types.splitdouble [[dvec]], 1
  ; CHECK: call void @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle [[anhdl]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 [[lodbl]], i32 [[hidbl]], i32 [[lodbl]], i32 [[hidbl]], i8 15, i32 0)

  %tmp218 = add <2 x i32> %ix2, <i32 38, i32 38>
  %tmp219 = load %"class.RWTexture2DMS<double, 0>", %"class.RWTexture2DMS<double, 0>"* @"\01?DTex2dMs@@3V?$RWTexture2DMS@N$0A@@@A"
  %tmp220 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<double, 0>\22)"(i32 0, %"class.RWTexture2DMS<double, 0>" %tmp219)
  %tmp221 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<double, 0>\22)"(i32 14, %dx.types.Handle %tmp220, %dx.types.ResourceProperties { i32 4099, i32 261 }, %"class.RWTexture2DMS<double, 0>" zeroinitializer)
  %tmp222 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp221, <2 x i32> %tmp218)
  %tmp223 = load double, double* %tmp222
  %tmp224 = add <2 x i32> %ix2, <i32 39, i32 39>
  %tmp225 = load %"class.RWTexture2DMS<double, 0>", %"class.RWTexture2DMS<double, 0>"* @"\01?DTex2dMs@@3V?$RWTexture2DMS@N$0A@@@A"
  %tmp226 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<double, 0>\22)"(i32 0, %"class.RWTexture2DMS<double, 0>" %tmp225)
  %tmp227 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<double, 0>\22)"(i32 14, %dx.types.Handle %tmp226, %dx.types.ResourceProperties { i32 4099, i32 261 }, %"class.RWTexture2DMS<double, 0>" zeroinitializer)
  %tmp228 = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %tmp227, <2 x i32> %tmp224)
  store double %tmp223, double* %tmp228

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<float, 3>, 0>"(i32 160, %"class.RWTexture2DMS<vector<float, 3>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 777 })
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 40, i32 40>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[anhdl]], i32 [[ix1]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[ping:%.*]] = insertelement <3 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <3 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[vec:%.*]] = insertelement <3 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[ix:%.*]] = add i32 [[ix1]], 1
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<float, 3>, 0>"(i32 160, %"class.RWTexture2DMS<vector<float, 3>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 777 })
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 41, i32 41>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[val3:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <3 x float> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <3 x float> [[vec]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <3 x float> [[vec]], i64 2
  ; CHECK: call void @dx.op.textureStoreSample.f32(i32 225, %dx.types.Handle %388, i32 %389, i32 %390, i32 undef, float %392, float %393, float %394, float %391, i8 15, i32 %tmp235)
  %tmp229 = load %"class.RWTexture2DMS<vector<float, 3>, 0>", %"class.RWTexture2DMS<vector<float, 3>, 0>"* @"\01?FTex2dMs@@3V?$RWTexture2DMS@V?$vector@M$02@@$0A@@@A"
  %tmp230 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<float, 3>, 0>" %tmp229)
  %tmp231 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 14, %dx.types.Handle %tmp230, %dx.types.ResourceProperties { i32 4099, i32 777 }, %"class.RWTexture2DMS<vector<float, 3>, 0>" zeroinitializer)
  %tmp232 = add <2 x i32> %ix2, <i32 40, i32 40>
  %tmp233 = call <3 x float>* @"dx.hl.subscript.[][].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle %tmp231, <2 x i32> %tmp232, i32 %ix1)
  %tmp234 = load <3 x float>, <3 x float>* %tmp233
  %tmp235 = add i32 %ix1, 1
  %tmp236 = load %"class.RWTexture2DMS<vector<float, 3>, 0>", %"class.RWTexture2DMS<vector<float, 3>, 0>"* @"\01?FTex2dMs@@3V?$RWTexture2DMS@V?$vector@M$02@@$0A@@@A"
  %tmp237 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<float, 3>, 0>" %tmp236)
  %tmp238 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 14, %dx.types.Handle %tmp237, %dx.types.ResourceProperties { i32 4099, i32 777 }, %"class.RWTexture2DMS<vector<float, 3>, 0>" zeroinitializer)
  %tmp239 = add <2 x i32> %ix2, <i32 41, i32 41>
  %tmp240 = call <3 x float>* @"dx.hl.subscript.[][].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle %tmp238, <2 x i32> %tmp239, i32 %tmp235)
  store <3 x float> %tmp234, <3 x float>* %tmp240

  ; CHECK: [[sax:%.*]] = add i32 [[ix1]], 2
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<bool, 2>, 0>"(i32 160, %"class.RWTexture2DMS<vector<bool, 2>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 })
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 42, i32 42>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 [[sax]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x i32> undef, i32 [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <2 x i32> [[ping]], i32 [[val1]], i64 1
  ; CHECK: %tmp248 = icmp ne <2 x i32> %402, zeroinitializer
  ; CHECK: [[sax:%.*]] = add i32 [[ix1]], 3
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<bool, 2>, 0>"(i32 160, %"class.RWTexture2DMS<vector<bool, 2>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 })
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 43, i32 43>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: %407 = extractelement <2 x i32> %tmp255, i64 0
  ; CHECK: %408 = extractelement <2 x i32> %tmp255, i64 0
  ; CHECK: %409 = extractelement <2 x i32> %tmp255, i64 1
  ; CHECK: call void @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle %404, i32 %405, i32 %406, i32 undef, i32 %408, i32 %409, i32 %407, i32 %407, i8 15, i32 %tmp249)
  ; CHECK: %tmp255 = zext <2 x i1> %tmp248 to <2 x i32>
  %tmp241 = add i32 %ix1, 2
  %tmp242 = load %"class.RWTexture2DMS<vector<bool, 2>, 0>", %"class.RWTexture2DMS<vector<bool, 2>, 0>"* @"\01?BTex2dMs@@3V?$RWTexture2DMS@V?$vector@_N$01@@$0A@@@A"
  %tmp243 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<bool, 2>, 0>" %tmp242)
  %tmp244 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle %tmp243, %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<bool, 2>, 0>" zeroinitializer)
  %tmp245 = add <2 x i32> %ix2, <i32 42, i32 42>
  %tmp246 = call <2 x i32>* @"dx.hl.subscript.[][].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle %tmp244, <2 x i32> %tmp245, i32 %tmp241)
  %tmp247 = load <2 x i32>, <2 x i32>* %tmp246
  %tmp248 = icmp ne <2 x i32> %tmp247, zeroinitializer
  %tmp249 = add i32 %ix1, 3
  %tmp250 = load %"class.RWTexture2DMS<vector<bool, 2>, 0>", %"class.RWTexture2DMS<vector<bool, 2>, 0>"* @"\01?BTex2dMs@@3V?$RWTexture2DMS@V?$vector@_N$01@@$0A@@@A"
  %tmp251 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<bool, 2>, 0>" %tmp250)
  %tmp252 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle %tmp251, %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<bool, 2>, 0>" zeroinitializer)
  %tmp253 = add <2 x i32> %ix2, <i32 43, i32 43>
  %tmp254 = call <2 x i32>* @"dx.hl.subscript.[][].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle %tmp252, <2 x i32> %tmp253, i32 %tmp249)
  %tmp255 = zext <2 x i1> %tmp248 to <2 x i32>
  store <2 x i32> %tmp255, <2 x i32>* %tmp254

  ; CHECK: [[sax:%.*]] = add i32 [[ix1]], 4
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"(i32 160, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 })
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 44, i32 44>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 [[sax]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 3
  ; CHECK: [[loval:%.*]] = zext i32 [[val0]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val1]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val0:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[loval:%.*]] = zext i32 [[val2]] to i64
  ; CHECK: [[hival:%.*]] = zext i32 [[val3]] to i64
  ; CHECK: [[val:%.*]] = shl i64 [[hival]], 32
  ; CHECK: [[val1:%.*]] = or i64 [[loval]], [[val]]
  ; CHECK: [[ping:%.*]] = insertelement <2 x i64> undef, i64 [[val0]], i64 0
  ; CHECK: [[vec:%.*]] = insertelement <2 x i64> [[ping]], i64 [[val1]], i64 1
  ; CHECK: [[sax:%.*]] = add i32 [[ix1]], 5
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"(i32 160, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 })
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 45, i32 45>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[val3:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val0:%.*]] = extractelement <2 x i64> [[vec]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i64> [[vec]], i64 1
  ; CHECK: [[loval0:%.*]] = trunc i64 [[val0]] to i32
  ; CHECK: [[msk0:%.*]] = lshr i64 [[val0]], 32
  ; CHECK: [[hival0:%.*]] = trunc i64 [[msk0]] to i32
  ; CHECK: [[loval1:%.*]] = trunc i64 [[val1]] to i32
  ; CHECK: [[msk1:%.*]] = lshr i64 [[val1]], 32
  ; CHECK: [[hival1:%.*]] = trunc i64 [[msk1]] to i32
  ; CHECK: call void @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle [[anhdl]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 [[loval0]], i32 [[hival0]], i32 [[loval1]], i32 [[hival1]], i8 15, i32 [[sax]])
  %tmp256 = add i32 %ix1, 4
  %tmp257 = load %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>", %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"* @"\01?LTex2dMs@@3V?$RWTexture2DMS@V?$vector@_K$01@@$0A@@@A"
  %tmp258 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" %tmp257)
  %tmp259 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 14, %dx.types.Handle %tmp258, %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" zeroinitializer)
  %tmp260 = add <2 x i32> %ix2, <i32 44, i32 44>
  %tmp261 = call <2 x i64>* @"dx.hl.subscript.[][].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle %tmp259, <2 x i32> %tmp260, i32 %tmp256)
  %tmp262 = load <2 x i64>, <2 x i64>* %tmp261
  %tmp263 = add i32 %ix1, 5
  %tmp264 = load %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>", %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"* @"\01?LTex2dMs@@3V?$RWTexture2DMS@V?$vector@_K$01@@$0A@@@A"
  %tmp265 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" %tmp264)
  %tmp266 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 14, %dx.types.Handle %tmp265, %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" zeroinitializer)
  %tmp267 = add <2 x i32> %ix2, <i32 45, i32 45>
  %tmp268 = call <2 x i64>* @"dx.hl.subscript.[][].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle %tmp266, <2 x i32> %tmp267, i32 %tmp263)
  store <2 x i64> %tmp262, <2 x i64>* %tmp268

  ; CHECK: [[sax:%.*]] = add i32 [[ix1]], 6
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<double, 0>"(i32 160, %"class.RWTexture2DMS<double, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 261 })
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 46, i32 46>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[anhdl]], i32 [[sax]], i32 [[ix2_0]], i32 [[ix2_1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: %447 = call double @dx.op.makeDouble.f64(i32 101, i32 %445, i32 %446)
  ; CHECK: [[sax:%.*]] = add i32 [[ix1]], 7
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2DMS<double, 0>"(i32 160, %"class.RWTexture2DMS<double, 0>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 261 })
  ; CHECK: [[ix:%.*]] = add <2 x i32> [[ix2]], <i32 47, i32 47>
  ; CHECK: [[ix2_0:%.*]] = extractelement <2 x i32> [[ix]], i64 0
  ; CHECK: [[ix2_1:%.*]] = extractelement <2 x i32> [[ix]], i64 1
  ; CHECK: %452 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %447)
  ; CHECK: %453 = extractvalue %dx.types.splitdouble %452, 0
  ; CHECK: %454 = extractvalue %dx.types.splitdouble %452, 1
  ; CHECK: call void @dx.op.textureStoreSample.i32(i32 225, %dx.types.Handle %449, i32 %450, i32 %451, i32 undef, i32 %453, i32 %454, i32 %453, i32 %454, i8 15, i32 %tmp276)
  %tmp269 = add i32 %ix1, 6
  %tmp270 = load %"class.RWTexture2DMS<double, 0>", %"class.RWTexture2DMS<double, 0>"* @"\01?DTex2dMs@@3V?$RWTexture2DMS@N$0A@@@A"
  %tmp271 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<double, 0>\22)"(i32 0, %"class.RWTexture2DMS<double, 0>" %tmp270)
  %tmp272 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<double, 0>\22)"(i32 14, %dx.types.Handle %tmp271, %dx.types.ResourceProperties { i32 4099, i32 261 }, %"class.RWTexture2DMS<double, 0>" zeroinitializer)
  %tmp273 = add <2 x i32> %ix2, <i32 46, i32 46>
  %tmp274 = call double* @"dx.hl.subscript.[][].rn.double* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle %tmp272, <2 x i32> %tmp273, i32 %tmp269)
  %tmp275 = load double, double* %tmp274
  %tmp276 = add i32 %ix1, 7
  %tmp277 = load %"class.RWTexture2DMS<double, 0>", %"class.RWTexture2DMS<double, 0>"* @"\01?DTex2dMs@@3V?$RWTexture2DMS@N$0A@@@A"
  %tmp278 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<double, 0>\22)"(i32 0, %"class.RWTexture2DMS<double, 0>" %tmp277)
  %tmp279 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<double, 0>\22)"(i32 14, %dx.types.Handle %tmp278, %dx.types.ResourceProperties { i32 4099, i32 261 }, %"class.RWTexture2DMS<double, 0>" zeroinitializer)
  %tmp280 = add <2 x i32> %ix2, <i32 47, i32 47>
  %tmp281 = call double* @"dx.hl.subscript.[][].rn.double* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle %tmp279, <2 x i32> %tmp280, i32 %tmp276)
  store double %tmp275, double* %tmp281


  ; CHECK: ret void
  ret void
}


declare <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 3> >\22)"(i32, %"class.RWBuffer<vector<float, 3> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 3> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWBuffer<vector<float, 3> >") #1
declare <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32, %"class.RWBuffer<vector<bool, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWBuffer<vector<bool, 2> >") #1
declare <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32, %"class.RWBuffer<vector<unsigned long long, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWBuffer<vector<unsigned long long, 2> >") #1
declare double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<double>\22)"(i32, %"class.RWBuffer<double>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<double>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWBuffer<double>") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32, %"class.RWTexture1D<vector<float, 3> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture1D<vector<float, 3> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32, %"class.RWTexture1D<vector<bool, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture1D<vector<bool, 2> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32, %"class.RWTexture1D<vector<unsigned long long, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture1D<vector<unsigned long long, 2> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<double>\22)"(i32, %"class.RWTexture1D<double>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<double>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture1D<double>") #1
declare <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32, %dx.types.Handle, <2 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32, %"class.RWTexture2D<vector<float, 3> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture2D<vector<float, 3> >") #1
declare <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32, %dx.types.Handle, <2 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32, %"class.RWTexture2D<vector<bool, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture2D<vector<bool, 2> >") #1
declare <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>)"(i32, %dx.types.Handle, <2 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32, %"class.RWTexture2D<vector<unsigned long long, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture2D<vector<unsigned long long, 2> >") #1
declare double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <2 x i32>)"(i32, %dx.types.Handle, <2 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<double>\22)"(i32, %"class.RWTexture2D<double>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<double>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture2D<double>") #1
declare <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32, %dx.types.Handle, <3 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32, %"class.RWTexture3D<vector<float, 3> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture3D<vector<float, 3> >") #1
declare <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <3 x i32>)"(i32, %dx.types.Handle, <3 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32, %"class.RWTexture3D<vector<bool, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture3D<vector<bool, 2> >") #1
declare <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <3 x i32>)"(i32, %dx.types.Handle, <3 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32, %"class.RWTexture3D<vector<unsigned long long, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture3D<vector<unsigned long long, 2> >") #1
declare double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <3 x i32>)"(i32, %dx.types.Handle, <3 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<double>\22)"(i32, %"class.RWTexture3D<double>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<double>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture3D<double>") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32, %"class.RWTexture2DMS<vector<float, 3>, 0>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture2DMS<vector<float, 3>, 0>") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32, %"class.RWTexture2DMS<vector<bool, 2>, 0>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture2DMS<vector<bool, 2>, 0>") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<double, 0>\22)"(i32, %"class.RWTexture2DMS<double, 0>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<double, 0>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture2DMS<double, 0>") #1
declare <3 x float>* @"dx.hl.subscript.[][].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32, %dx.types.Handle, <2 x i32>, i32) #1
declare <2 x i32>* @"dx.hl.subscript.[][].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32, %dx.types.Handle, <2 x i32>, i32) #1
declare <2 x i64>* @"dx.hl.subscript.[][].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32, %dx.types.Handle, <2 x i32>, i32) #1
declare double* @"dx.hl.subscript.[][].rn.double* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32, %dx.types.Handle, <2 x i32>, i32) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!19}
!dx.fnprops = !{!44}
!dx.options = !{!45, !46}

!3 = !{i32 1, i32 6}
!4 = !{i32 1, i32 9}
!5 = !{!"vs", i32 6, i32 6}
!6 = !{i32 1, void (i32, <2 x i32>, <3 x i32>)* @main, !7}
!7 = !{!8, !10, !13, !16}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{i32 0, !11, !12}
!11 = !{i32 4, !"IX1", i32 7, i32 5}
!12 = !{i32 1}
!13 = !{i32 0, !14, !15}
!14 = !{i32 4, !"IX2", i32 7, i32 5}
!15 = !{i32 2}
!16 = !{i32 0, !17, !18}
!17 = !{i32 4, !"IX3", i32 7, i32 5}
!18 = !{i32 3}
!19 = !{void (i32, <2 x i32>, <3 x i32>)* @main, !"main", null, !20, null}
!20 = !{null, !21, null, null}
!21 = !{!22, !24, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43}
!22 = !{i32 0, %"class.RWBuffer<vector<float, 3> >"* @"\01?FTyBuf@@3V?$RWBuffer@V?$vector@M$02@@@@A", !"FTyBuf", i32 -1, i32 -1, i32 1, i32 10, i1 false, i1 false, i1 false, !23}
!23 = !{i32 0, i32 9}
!24 = !{i32 1, %"class.RWBuffer<vector<bool, 2> >"* @"\01?BTyBuf@@3V?$RWBuffer@V?$vector@_N$01@@@@A", !"BTyBuf", i32 -1, i32 -1, i32 1, i32 10, i1 false, i1 false, i1 false, !25}
!25 = !{i32 0, i32 5}
!26 = !{i32 2, %"class.RWBuffer<vector<unsigned long long, 2> >"* @"\01?LTyBuf@@3V?$RWBuffer@V?$vector@_K$01@@@@A", !"LTyBuf", i32 -1, i32 -1, i32 1, i32 10, i1 false, i1 false, i1 false, !25}
!27 = !{i32 3, %"class.RWBuffer<double>"* @"\01?DTyBuf@@3V?$RWBuffer@N@@A", !"DTyBuf", i32 -1, i32 -1, i32 1, i32 10, i1 false, i1 false, i1 false, !25}
!28 = !{i32 4, %"class.RWTexture1D<vector<float, 3> >"* @"\01?FTex1d@@3V?$RWTexture1D@V?$vector@M$02@@@@A", !"FTex1d", i32 -1, i32 -1, i32 1, i32 1, i1 false, i1 false, i1 false, !23}
!29 = !{i32 5, %"class.RWTexture1D<vector<bool, 2> >"* @"\01?BTex1d@@3V?$RWTexture1D@V?$vector@_N$01@@@@A", !"BTex1d", i32 -1, i32 -1, i32 1, i32 1, i1 false, i1 false, i1 false, !25}
!30 = !{i32 6, %"class.RWTexture1D<vector<unsigned long long, 2> >"* @"\01?LTex1d@@3V?$RWTexture1D@V?$vector@_K$01@@@@A", !"LTex1d", i32 -1, i32 -1, i32 1, i32 1, i1 false, i1 false, i1 false, !25}
!31 = !{i32 7, %"class.RWTexture1D<double>"* @"\01?DTex1d@@3V?$RWTexture1D@N@@A", !"DTex1d", i32 -1, i32 -1, i32 1, i32 1, i1 false, i1 false, i1 false, !25}
!32 = !{i32 8, %"class.RWTexture2D<vector<float, 3> >"* @"\01?FTex2d@@3V?$RWTexture2D@V?$vector@M$02@@@@A", !"FTex2d", i32 -1, i32 -1, i32 1, i32 2, i1 false, i1 false, i1 false, !23}
!33 = !{i32 9, %"class.RWTexture2D<vector<bool, 2> >"* @"\01?BTex2d@@3V?$RWTexture2D@V?$vector@_N$01@@@@A", !"BTex2d", i32 -1, i32 -1, i32 1, i32 2, i1 false, i1 false, i1 false, !25}
!34 = !{i32 10, %"class.RWTexture2D<vector<unsigned long long, 2> >"* @"\01?LTex2d@@3V?$RWTexture2D@V?$vector@_K$01@@@@A", !"LTex2d", i32 -1, i32 -1, i32 1, i32 2, i1 false, i1 false, i1 false, !25}
!35 = !{i32 11, %"class.RWTexture2D<double>"* @"\01?DTex2d@@3V?$RWTexture2D@N@@A", !"DTex2d", i32 -1, i32 -1, i32 1, i32 2, i1 false, i1 false, i1 false, !25}
!36 = !{i32 12, %"class.RWTexture3D<vector<float, 3> >"* @"\01?FTex3d@@3V?$RWTexture3D@V?$vector@M$02@@@@A", !"FTex3d", i32 -1, i32 -1, i32 1, i32 4, i1 false, i1 false, i1 false, !23}
!37 = !{i32 13, %"class.RWTexture3D<vector<bool, 2> >"* @"\01?BTex3d@@3V?$RWTexture3D@V?$vector@_N$01@@@@A", !"BTex3d", i32 -1, i32 -1, i32 1, i32 4, i1 false, i1 false, i1 false, !25}
!38 = !{i32 14, %"class.RWTexture3D<vector<unsigned long long, 2> >"* @"\01?LTex3d@@3V?$RWTexture3D@V?$vector@_K$01@@@@A", !"LTex3d", i32 -1, i32 -1, i32 1, i32 4, i1 false, i1 false, i1 false, !25}
!39 = !{i32 15, %"class.RWTexture3D<double>"* @"\01?DTex3d@@3V?$RWTexture3D@N@@A", !"DTex3d", i32 -1, i32 -1, i32 1, i32 4, i1 false, i1 false, i1 false, !25}
!40 = !{i32 16, %"class.RWTexture2DMS<vector<float, 3>, 0>"* @"\01?FTex2dMs@@3V?$RWTexture2DMS@V?$vector@M$02@@$0A@@@A", !"FTex2dMs", i32 -1, i32 -1, i32 1, i32 3, i1 false, i1 false, i1 false, !23}
!41 = !{i32 17, %"class.RWTexture2DMS<vector<bool, 2>, 0>"* @"\01?BTex2dMs@@3V?$RWTexture2DMS@V?$vector@_N$01@@$0A@@@A", !"BTex2dMs", i32 -1, i32 -1, i32 1, i32 3, i1 false, i1 false, i1 false, !25}
!42 = !{i32 18, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"* @"\01?LTex2dMs@@3V?$RWTexture2DMS@V?$vector@_K$01@@$0A@@@A", !"LTex2dMs", i32 -1, i32 -1, i32 1, i32 3, i1 false, i1 false, i1 false, !25}
!43 = !{i32 19, %"class.RWTexture2DMS<double, 0>"* @"\01?DTex2dMs@@3V?$RWTexture2DMS@N$0A@@@A", !"DTex2dMs", i32 -1, i32 -1, i32 1, i32 3, i1 false, i1 false, i1 false, !25}
!44 = !{void (i32, <2 x i32>, <3 x i32>)* @main, i32 1}
!45 = !{i32 64}
!46 = !{i32 -1}
