; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RWByteAddressBuffer = type { i32 }
%"class.RWStructuredBuffer<vector<float, 2> >" = type { <2 x float> }
%"class.RWStructuredBuffer<float [2]>" = type { [2 x float] }
%"class.RWStructuredBuffer<Vector<float, 2> >" = type { %"struct.Vector<float, 2>" }
%"struct.Vector<float, 2>" = type { <4 x float>, double, <2 x float> }
%"class.RWStructuredBuffer<matrix<float, 2, 2> >" = type { %class.matrix.float.2.2 }
%class.matrix.float.2.2 = type { [2 x <2 x float>] }
%"class.RWStructuredBuffer<Matrix<float, 2, 2> >" = type { %"struct.Matrix<float, 2, 2>" }
%"struct.Matrix<float, 2, 2>" = type { <4 x float>, %class.matrix.float.2.2 }
%"class.ConsumeStructuredBuffer<vector<float, 2> >" = type { <2 x float> }
%"class.ConsumeStructuredBuffer<float [2]>" = type { [2 x float] }
%"class.ConsumeStructuredBuffer<Vector<float, 2> >" = type { %"struct.Vector<float, 2>" }
%"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >" = type { %class.matrix.float.2.2 }
%"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >" = type { %"struct.Matrix<float, 2, 2>" }
%"class.AppendStructuredBuffer<vector<float, 2> >" = type { <2 x float> }
%"class.AppendStructuredBuffer<float [2]>" = type { [2 x float] }
%"class.AppendStructuredBuffer<Vector<float, 2> >" = type { %"struct.Vector<float, 2>" }
%"class.AppendStructuredBuffer<matrix<float, 2, 2> >" = type { %class.matrix.float.2.2 }
%"class.AppendStructuredBuffer<Matrix<float, 2, 2> >" = type { %"struct.Matrix<float, 2, 2>" }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?BabBuf@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4
@"\01?VecBuf@@3V?$RWStructuredBuffer@V?$vector@M$01@@@@A" = external global %"class.RWStructuredBuffer<vector<float, 2> >", align 4
@"\01?ArrBuf@@3V?$RWStructuredBuffer@$$BY01M@@A" = external global %"class.RWStructuredBuffer<float [2]>", align 4
@"\01?SVecBuf@@3V?$RWStructuredBuffer@U?$Vector@M$01@@@@A" = external global %"class.RWStructuredBuffer<Vector<float, 2> >", align 8
@"\01?MatBuf@@3V?$RWStructuredBuffer@V?$matrix@M$01$01@@@@A" = external global %"class.RWStructuredBuffer<matrix<float, 2, 2> >", align 4
@"\01?SMatBuf@@3V?$RWStructuredBuffer@U?$Matrix@M$01$01@@@@A" = external global %"class.RWStructuredBuffer<Matrix<float, 2, 2> >", align 4
@"\01?CVecBuf@@3V?$ConsumeStructuredBuffer@V?$vector@M$01@@@@A" = external global %"class.ConsumeStructuredBuffer<vector<float, 2> >", align 4
@"\01?CArrBuf@@3V?$ConsumeStructuredBuffer@$$BY01M@@A" = external global %"class.ConsumeStructuredBuffer<float [2]>", align 4
@"\01?CSVecBuf@@3V?$ConsumeStructuredBuffer@U?$Vector@M$01@@@@A" = external global %"class.ConsumeStructuredBuffer<Vector<float, 2> >", align 8
@"\01?CMatBuf@@3V?$ConsumeStructuredBuffer@V?$matrix@M$01$01@@@@A" = external global %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >", align 4
@"\01?CSMatBuf@@3V?$ConsumeStructuredBuffer@U?$Matrix@M$01$01@@@@A" = external global %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >", align 4
@"\01?AVecBuf@@3V?$AppendStructuredBuffer@V?$vector@M$01@@@@A" = external global %"class.AppendStructuredBuffer<vector<float, 2> >", align 4
@"\01?AArrBuf@@3V?$AppendStructuredBuffer@$$BY01M@@A" = external global %"class.AppendStructuredBuffer<float [2]>", align 4
@"\01?ASVecBuf@@3V?$AppendStructuredBuffer@U?$Vector@M$01@@@@A" = external global %"class.AppendStructuredBuffer<Vector<float, 2> >", align 8
@"\01?AMatBuf@@3V?$AppendStructuredBuffer@V?$matrix@M$01$01@@@@A" = external global %"class.AppendStructuredBuffer<matrix<float, 2, 2> >", align 4
@"\01?ASMatBuf@@3V?$AppendStructuredBuffer@U?$Matrix@M$01$01@@@@A" = external global %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >", align 4

; CHECK-LABEL: define void @main(i32 %ix0)
; Function Attrs: nounwind
define void @main(i32 %ix0) #0 {
bb:
  ; CHECK: [[pix:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle [[anhdl]], i32 [[pix]], i32 undef, i8 3, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.i32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x i32> undef, i32 [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <2 x i32> [[ping]], i32 [[val1]], i64 1
  ; CHECK: [[bvec:%.*]] = icmp ne <2 x i32> [[pong]], zeroinitializer

  %tmp = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:60 col:32
  %tmp1 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp) ; line:60 col:32
  %tmp2 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:60 col:32
  %tmp3 = call <2 x i1> @"dx.hl.op.ro.<2 x i1> (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %tmp2, i32 %ix0) ; line:60 col:32

  ; CHECK: [[stix:%.*]] = add i32 [[pix]], 1
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: [[vec2:%.*]] = zext <2 x i1> [[bvec]] to <2 x i32>
  ; CHECK: [[val0:%.*]] = extractelement <2 x i32> [[vec2]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x i32> [[vec2]], i64 1
  ; CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle [[anhdl]], i32 [[stix]], i32 undef, i32 [[val0]], i32 [[val1]], i32 undef, i32 undef, i8 3, i32 4)
  %tmp4 = add i32 %ix0, 1 ; line:60 col:27
  %tmp5 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:60 col:3
  %tmp6 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp5) ; line:60 col:3
  %tmp7 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp6, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:60 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <2 x i1>)"(i32 277, %dx.types.Handle %tmp7, i32 %tmp4, <2 x i1> %tmp3) ; line:60 col:3

  ; CHECK: [[ix:%.*]] = add i32 [[pix]], 1
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: [[lix:%.*]] = add i32 0, [[ix]]
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[lix]], i32 undef, i8 1, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[lix:%.*]] = add i32 4, [[ix]]
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[lix]], i32 undef, i8 1, i32 4)
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0

  %tmp8 = add i32 %ix0, 1 ; line:70 col:63
  %tmp9 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:70 col:35
  %tmp10 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp9) ; line:70 col:35
  %tmp11 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp10, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:70 col:35
  %tmp12 = call [2 x float]* @"dx.hl.op.ro.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %tmp11, i32 %tmp8) ; line:70 col:35
  %tmp13 = getelementptr inbounds [2 x float], [2 x float]* %tmp12, i32 0, i32 0 ; line:70 col:3
  %tmp14 = load float, float* %tmp13 ; line:70 col:3
  %tmp15 = getelementptr inbounds [2 x float], [2 x float]* %tmp12, i32 0, i32 1 ; line:70 col:3
  %tmp16 = load float, float* %tmp15 ; line:70 col:3


  ; CHECK: [[ix:%.*]] = add i32 [[pix]], 2
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, float [[val0]], float undef, float undef, float undef, i8 1, i32 4)
  ; CHECK: [[stix:%.*]] = add i32 [[ix]], 4
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[stix]], i32 undef, float [[val1]], float undef, float undef, float undef, i8 1, i32 4)

  %tmp17 = add i32 %ix0, 2 ; line:70 col:30
  %tmp18 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:70 col:3
  %tmp19 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp18) ; line:70 col:3
  %tmp20 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp19, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:70 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32 277, %dx.types.Handle %tmp20, i32 %tmp17, float %tmp14) ; line:70 col:3
  %tmp21 = add i32 %tmp17, 4 ; line:70 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32 277, %dx.types.Handle %tmp20, i32 %tmp21, float %tmp16) ; line:70 col:3

  ; CHECK: [[ix:%.*]] = add i32 [[pix]], 2
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: [[lix:%.*]] = add i32 0, [[ix]]
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[lix]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[vec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  ; CHECK: [[lix:%.*]] = add i32 16, [[ix]]
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle [[anhdl]], i32 [[lix]], i32 undef, i8 1, i32 4)
  ; CHECK: [[dval:%.*]] = extractvalue %dx.types.ResRet.f64 [[ld]], 0
  ; CHECK: [[lix:%.*]] = add i32 24, [[ix]]
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[lix]], i32 undef, i8 3, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec2:%.*]] = insertelement <2 x float> [[ping]], float [[val1]], i64 1
  %tmp22 = add i32 %ix0, 2 ; line:80 col:78
  %tmp23 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:80 col:43
  %tmp24 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp23) ; line:80 col:43
  %tmp25 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp24, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:80 col:43
  %tmp26 = call %"struct.Vector<float, 2>"* @"dx.hl.op.ro.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %tmp25, i32 %tmp22) ; line:80 col:43
  %tmp27 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp26, i32 0, i32 0 ; line:80 col:3
  %tmp28 = load <4 x float>, <4 x float>* %tmp27 ; line:80 col:3
  %tmp29 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp26, i32 0, i32 1 ; line:80 col:3
  %tmp30 = load double, double* %tmp29 ; line:80 col:3
  %tmp31 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp26, i32 0, i32 2 ; line:80 col:3
  %tmp32 = load <2 x float>, <2 x float>* %tmp31 ; line:80 col:3

  ; CHECK: [[ix:%.*]] = add i32 [[pix]], 3
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[vec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[vec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[vec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[vec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  ; CHECK: [[stix:%.*]] = add i32 [[ix]], 16
  ; CHECK: call void @dx.op.rawBufferStore.f64(i32 140, %dx.types.Handle [[anhdl]], i32 [[stix]], i32 undef, double [[dval]]
  ; CHECK: [[stix:%.*]] = add i32 [[ix]], 24
  ; CHECK: [[val0:%.*]] = extractelement <2 x float> [[vec2]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x float> [[vec2]], i64 1
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[stix]], i32 undef, float [[val0]], float [[val1]], float undef, float undef, i8 3, i32 4)
  %tmp33 = add i32 %ix0, 3 ; line:80 col:38
  %tmp34 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:80 col:3
  %tmp35 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp34) ; line:80 col:3
  %tmp36 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp35, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:80 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <4 x float>)"(i32 277, %dx.types.Handle %tmp36, i32 %tmp33, <4 x float> %tmp28) ; line:80 col:3
  %tmp37 = add i32 %tmp33, 16 ; line:80 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, double)"(i32 277, %dx.types.Handle %tmp36, i32 %tmp37, double %tmp30) ; line:80 col:3
  %tmp38 = add i32 %tmp33, 24 ; line:80 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <2 x float>)"(i32 277, %dx.types.Handle %tmp36, i32 %tmp38, <2 x float> %tmp32) ; line:80 col:3


  ; CHECK: [[lix:%.*]] = add i32 [[pix]], 3
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[lix]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[rvec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  %tmp39 = add i32 %ix0, 3 ; line:90 col:63
  %tmp40 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:90 col:35
  %tmp41 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp40) ; line:90 col:35
  %tmp42 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp41, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:90 col:35
  %tmp43 = call <4 x float> @"dx.hl.op.ro.<4 x float> (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %tmp42, i32 %tmp39) ; line:90 col:35

  ; CHECK: [[stix:%.*]] = add i32 [[pix]], 4
  ; CHECK: [[cvec4:%.*]] = shufflevector <4 x float> [[rvec4]], <4 x float> [[rvec4]], <4 x i32> <i32 0, i32 2, i32 1, i32 3>
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[cvec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[cvec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[cvec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[cvec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[stix]], i32 undef, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  %tmp44 = add i32 %ix0, 4 ; line:90 col:30
  %row2col = shufflevector <4 x float> %tmp43, <4 x float> %tmp43, <4 x i32> <i32 0, i32 2, i32 1, i32 3> ; line:90 col:3
  %tmp45 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:90 col:3
  %tmp46 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp45) ; line:90 col:3
  %tmp47 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp46, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:90 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <4 x float>)"(i32 277, %dx.types.Handle %tmp47, i32 %tmp44, <4 x float> %row2col) ; line:90 col:3


  ; CHECK: [[ix:%.*]] = add i32 [[pix]], 4
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: [[lix:%.*]] = add i32 0, [[ix]]
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[lix]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[vec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  ; CHECK: [[lix:%.*]] = add i32 16, [[ix]]
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[lix]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[mat:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  %tmp48 = add i32 %ix0, 4 ; line:100 col:82
  %tmp49 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:100 col:45
  %tmp50 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp49) ; line:100 col:45
  %tmp51 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp50, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:100 col:45
  %tmp52 = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.op.ro.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %tmp51, i32 %tmp48) ; line:100 col:45
  %tmp53 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp52, i32 0, i32 0 ; line:100 col:3
  %tmp54 = load <4 x float>, <4 x float>* %tmp53 ; line:100 col:3
  %tmp55 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp52, i32 0, i32 1 ; line:100 col:3
  %tmp56 = call <4 x float> @"dx.hl.matldst.colLoad.<4 x float> (i32, %class.matrix.float.2.2*)"(i32 0, %class.matrix.float.2.2* %tmp55) ; line:100 col:3

  ; CHECK: [[ix:%.*]] = add i32 [[pix]], 5
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[vec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[vec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[vec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[vec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ix]], i32 undef, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  ; CHECK: [[stix:%.*]] = add i32 [[ix]], 16
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[mat]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[mat]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[mat]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[mat]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[stix]], i32 undef, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  %tmp57 = add i32 %ix0, 5 ; line:100 col:40
  %tmp58 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A" ; line:100 col:3
  %tmp59 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp58) ; line:100 col:3
  %tmp60 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp59, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer) ; line:100 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <4 x float>)"(i32 277, %dx.types.Handle %tmp60, i32 %tmp57, <4 x float> %tmp54) ; line:100 col:3
  %tmp61 = add i32 %tmp57, 16 ; line:100 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <4 x float>)"(i32 277, %dx.types.Handle %tmp60, i32 %tmp61, <4 x float> %tmp56) ; line:100 col:3

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<vector<float, 2> >"(i32 160, %"class.RWStructuredBuffer<vector<float, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[pix]], i32 0, i8 3, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec2:%.*]] = insertelement <2 x float> [[ping]], float [[val1]], i64 1
  %tmp62 = load %"class.RWStructuredBuffer<vector<float, 2> >", %"class.RWStructuredBuffer<vector<float, 2> >"* @"\01?VecBuf@@3V?$RWStructuredBuffer@V?$vector@M$01@@@@A" ; line:111 col:21
  %tmp63 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 2> >" %tmp62) ; line:111 col:21
  %tmp64 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %tmp63, %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<vector<float, 2> >" zeroinitializer) ; line:111 col:21
  %tmp65 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp64, i32 %ix0) ; line:111 col:21
  %tmp66 = load <2 x float>, <2 x float>* %tmp65 ; line:111 col:21

  ; CHECK: [[stix:%.*]] = add i32 [[pix]], 1
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<vector<float, 2> >"(i32 160, %"class.RWStructuredBuffer<vector<float, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 })
  ; CHECK: [[val0:%.*]] = extractelement <2 x float> [[vec2]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x float> [[vec2]], i64 1
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[stix]], i32 0, float [[val0]], float [[val1]], float undef, float undef, i8 3, i32 4)
  %tmp67 = add i32 %ix0, 1 ; line:111 col:14
  %tmp68 = load %"class.RWStructuredBuffer<vector<float, 2> >", %"class.RWStructuredBuffer<vector<float, 2> >"* @"\01?VecBuf@@3V?$RWStructuredBuffer@V?$vector@M$01@@@@A" ; line:111 col:3
  %tmp69 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 2> >" %tmp68) ; line:111 col:3
  %tmp70 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %tmp69, %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<vector<float, 2> >" zeroinitializer) ; line:111 col:3
  %tmp71 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp70, i32 %tmp67) ; line:111 col:3
  store <2 x float> %tmp66, <2 x float>* %tmp71 ; line:111 col:19


  ; CHECK: [[stix:%.*]] = add i32 [[pix]], 2
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<float [2]>"(i32 160, %"class.RWStructuredBuffer<float [2]>"
  ; CHECK: [[sthdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 })
  ; CHECK: [[lix:%.*]] = add i32 [[pix]], 1
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<float [2]>"(i32 160, %"class.RWStructuredBuffer<float [2]>"
  ; CHECK: [[ldhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 })

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[ldhdl]], i32 [[lix]], i32 0, i8 1, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[sthdl]], i32 [[stix]], i32 0, float [[val0]], float undef, float undef, float undef, i8 1, i32 4)
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[ldhdl]], i32 [[lix]], i32 4, i8 1, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[sthdl]], i32 [[stix]], i32 4, float [[val0]], float undef, float undef, float undef, i8 1, i32 4)
  %tmp72 = add i32 %ix0, 2 ; line:121 col:14
  %tmp73 = load %"class.RWStructuredBuffer<float [2]>", %"class.RWStructuredBuffer<float [2]>"* @"\01?ArrBuf@@3V?$RWStructuredBuffer@$$BY01M@@A" ; line:121 col:3
  %tmp74 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<float [2]>\22)"(i32 0, %"class.RWStructuredBuffer<float [2]>" %tmp73) ; line:121 col:3
  %tmp75 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle %tmp74, %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<float [2]>" zeroinitializer) ; line:121 col:3
  %tmp76 = call [2 x float]* @"dx.hl.subscript.[].rn.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp75, i32 %tmp72) ; line:121 col:3
  %tmp77 = add i32 %ix0, 1 ; line:121 col:32
  %tmp78 = load %"class.RWStructuredBuffer<float [2]>", %"class.RWStructuredBuffer<float [2]>"* @"\01?ArrBuf@@3V?$RWStructuredBuffer@$$BY01M@@A" ; line:121 col:21
  %tmp79 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<float [2]>\22)"(i32 0, %"class.RWStructuredBuffer<float [2]>" %tmp78) ; line:121 col:21
  %tmp80 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle %tmp79, %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<float [2]>" zeroinitializer) ; line:121 col:21
  %tmp81 = call [2 x float]* @"dx.hl.subscript.[].rn.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp80, i32 %tmp77) ; line:121 col:21
  %tmp82 = getelementptr inbounds [2 x float], [2 x float]* %tmp76, i32 0, i32 0 ; line:121 col:21
  %tmp83 = getelementptr inbounds [2 x float], [2 x float]* %tmp81, i32 0, i32 0 ; line:121 col:21
  %tmp84 = load float, float* %tmp83 ; line:121 col:21
  store float %tmp84, float* %tmp82 ; line:121 col:21
  %tmp85 = getelementptr inbounds [2 x float], [2 x float]* %tmp76, i32 0, i32 1 ; line:121 col:21
  %tmp86 = getelementptr inbounds [2 x float], [2 x float]* %tmp81, i32 0, i32 1 ; line:121 col:21
  %tmp87 = load float, float* %tmp86 ; line:121 col:21
  store float %tmp87, float* %tmp85 ; line:121 col:21


  ; CHECK: [[stix:%.*]] = add i32 [[pix]], 3
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<Vector<float, 2> >"(i32 160, %"class.RWStructuredBuffer<Vector<float, 2> >"
  ; CHECK: [[sthdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4876, i32 32 })
  ; CHECK: [[lix:%.*]] = add i32 [[pix]], 2
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<Vector<float, 2> >"(i32 160, %"class.RWStructuredBuffer<Vector<float, 2> >"
  ; CHECK: [[ldhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4876, i32 32 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[ldhdl]], i32 [[lix]], i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[vec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[vec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[vec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[vec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[vec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[sthdl]], i32 [[stix]], i32 0, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle [[ldhdl]], i32 [[lix]], i32 16, i8 1, i32 8)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f64 [[ld]], 0
  ; CHECK: call void @dx.op.rawBufferStore.f64(i32 140, %dx.types.Handle [[sthdl]], i32 [[stix]], i32 16, double [[val0]], double undef, double undef, double undef, i8 1, i32 8)
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[ldhdl]], i32 [[lix]], i32 24, i8 3, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec2:%.*]] = insertelement <2 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[val0:%.*]] = extractelement <2 x float> [[vec2]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x float> [[vec2]], i64 1
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[sthdl]], i32 [[stix]], i32 24, float [[val0]], float [[val1]], float undef, float undef, i8 3, i32 4)
  %tmp88 = add i32 %ix0, 3 ; line:131 col:15
  %tmp89 = load %"class.RWStructuredBuffer<Vector<float, 2> >", %"class.RWStructuredBuffer<Vector<float, 2> >"* @"\01?SVecBuf@@3V?$RWStructuredBuffer@U?$Vector@M$01@@@@A" ; line:131 col:3
  %tmp90 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<Vector<float, 2> >" %tmp89) ; line:131 col:3
  %tmp91 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle %tmp90, %dx.types.ResourceProperties { i32 4876, i32 32 }, %"class.RWStructuredBuffer<Vector<float, 2> >" zeroinitializer) ; line:131 col:3
  %tmp92 = call %"struct.Vector<float, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp91, i32 %tmp88) ; line:131 col:3
  %tmp93 = add i32 %ix0, 2 ; line:131 col:34
  %tmp94 = load %"class.RWStructuredBuffer<Vector<float, 2> >", %"class.RWStructuredBuffer<Vector<float, 2> >"* @"\01?SVecBuf@@3V?$RWStructuredBuffer@U?$Vector@M$01@@@@A" ; line:131 col:22
  %tmp95 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<Vector<float, 2> >" %tmp94) ; line:131 col:22
  %tmp96 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle %tmp95, %dx.types.ResourceProperties { i32 4876, i32 32 }, %"class.RWStructuredBuffer<Vector<float, 2> >" zeroinitializer) ; line:131 col:22
  %tmp97 = call %"struct.Vector<float, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp96, i32 %tmp93) ; line:131 col:22
  %tmp98 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp92, i32 0, i32 0 ; line:131 col:22
  %tmp99 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp97, i32 0, i32 0 ; line:131 col:22
  %tmp100 = load <4 x float>, <4 x float>* %tmp99 ; line:131 col:22
  store <4 x float> %tmp100, <4 x float>* %tmp98 ; line:131 col:22
  %tmp101 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp92, i32 0, i32 1 ; line:131 col:22
  %tmp102 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp97, i32 0, i32 1 ; line:131 col:22
  %tmp103 = load double, double* %tmp102 ; line:131 col:22
  store double %tmp103, double* %tmp101 ; line:131 col:22
  %tmp104 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp92, i32 0, i32 2 ; line:131 col:22
  %tmp105 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp97, i32 0, i32 2 ; line:131 col:22
  %tmp106 = load <2 x float>, <2 x float>* %tmp105 ; line:131 col:22
  store <2 x float> %tmp106, <2 x float>* %tmp104 ; line:131 col:22


  ; CHECK: [[stix:%.*]] = add i32 [[pix]], 4
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<matrix<float, 2, 2> >"(i32 160, %"class.RWStructuredBuffer<matrix<float, 2, 2> >"
  ; CHECK: [[sthdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 16 })
  ; CHECK: [[lix:%.*]] = add i32 [[pix]], 3
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<matrix<float, 2, 2> >"(i32 160, %"class.RWStructuredBuffer<matrix<float, 2, 2> >"
  ; CHECK: [[ldhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 16 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[ldhdl]], i32 [[lix]], i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[vec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[vec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[vec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[vec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[vec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[sthdl]], i32 [[stix]], i32 0, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  %tmp107 = add i32 %ix0, 4 ; line:141 col:14
  %tmp108 = load %"class.RWStructuredBuffer<matrix<float, 2, 2> >", %"class.RWStructuredBuffer<matrix<float, 2, 2> >"* @"\01?MatBuf@@3V?$RWStructuredBuffer@V?$matrix@M$01$01@@@@A" ; line:141 col:3
  %tmp109 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<matrix<float, 2, 2> >" %tmp108) ; line:141 col:3
  %tmp110 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle %tmp109, %dx.types.ResourceProperties { i32 4620, i32 16 }, %"class.RWStructuredBuffer<matrix<float, 2, 2> >" zeroinitializer) ; line:141 col:3
  %tmp111 = call %class.matrix.float.2.2* @"dx.hl.subscript.[].rn.%class.matrix.float.2.2* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp110, i32 %tmp107) ; line:141 col:3
  %tmp112 = add i32 %ix0, 3 ; line:141 col:32
  %tmp113 = load %"class.RWStructuredBuffer<matrix<float, 2, 2> >", %"class.RWStructuredBuffer<matrix<float, 2, 2> >"* @"\01?MatBuf@@3V?$RWStructuredBuffer@V?$matrix@M$01$01@@@@A" ; line:141 col:21
  %tmp114 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<matrix<float, 2, 2> >" %tmp113) ; line:141 col:21
  %tmp115 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle %tmp114, %dx.types.ResourceProperties { i32 4620, i32 16 }, %"class.RWStructuredBuffer<matrix<float, 2, 2> >" zeroinitializer) ; line:141 col:21
  %tmp116 = call %class.matrix.float.2.2* @"dx.hl.subscript.[].rn.%class.matrix.float.2.2* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp115, i32 %tmp112) ; line:141 col:21
  %tmp117 = call <4 x float> @"dx.hl.matldst.colLoad.<4 x float> (i32, %class.matrix.float.2.2*)"(i32 0, %class.matrix.float.2.2* %tmp116) ; line:141 col:21
  %tmp118 = call <4 x float> @"dx.hl.matldst.colStore.<4 x float> (i32, %class.matrix.float.2.2*, <4 x float>)"(i32 1, %class.matrix.float.2.2* %tmp111, <4 x float> %tmp117) ; line:141 col:19


  ; CHECK: [[stix:%.*]] = add i32 [[pix]], 5
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<Matrix<float, 2, 2> >"(i32 160, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >"
  ; CHECK: [[sthdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 32 })
  ; CHECK: [[lix:%.*]] = add i32 [[pix]], 4
  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWStructuredBuffer<Matrix<float, 2, 2> >"(i32 160, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >"
  ; CHECK: [[ldhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 32 })
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[ldhdl]], i32 [[lix]], i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[vec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[vec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[vec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[vec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[vec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[sthdl]], i32 [[stix]], i32 0, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[ldhdl]], i32 [[lix]], i32 16, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[vec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[vec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[vec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[vec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[vec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[sthdl]], i32 [[stix]], i32 16, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  %tmp119 = add i32 %ix0, 5 ; line:151 col:15
  %tmp120 = load %"class.RWStructuredBuffer<Matrix<float, 2, 2> >", %"class.RWStructuredBuffer<Matrix<float, 2, 2> >"* @"\01?SMatBuf@@3V?$RWStructuredBuffer@U?$Matrix@M$01$01@@@@A" ; line:151 col:3
  %tmp121 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >" %tmp120) ; line:151 col:3
  %tmp122 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle %tmp121, %dx.types.ResourceProperties { i32 4620, i32 32 }, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >" zeroinitializer) ; line:151 col:3
  %tmp123 = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp122, i32 %tmp119) ; line:151 col:3
  %tmp124 = add i32 %ix0, 4 ; line:151 col:34
  %tmp125 = load %"class.RWStructuredBuffer<Matrix<float, 2, 2> >", %"class.RWStructuredBuffer<Matrix<float, 2, 2> >"* @"\01?SMatBuf@@3V?$RWStructuredBuffer@U?$Matrix@M$01$01@@@@A" ; line:151 col:22
  %tmp126 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >" %tmp125) ; line:151 col:22
  %tmp127 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle %tmp126, %dx.types.ResourceProperties { i32 4620, i32 32 }, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >" zeroinitializer) ; line:151 col:22
  %tmp128 = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp127, i32 %tmp124) ; line:151 col:22
  %tmp129 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp123, i32 0, i32 0 ; line:151 col:22
  %tmp130 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp128, i32 0, i32 0 ; line:151 col:22
  %tmp131 = load <4 x float>, <4 x float>* %tmp130 ; line:151 col:22
  store <4 x float> %tmp131, <4 x float>* %tmp129 ; line:151 col:22
  %tmp132 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp123, i32 0, i32 1 ; line:151 col:22
  %tmp133 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp128, i32 0, i32 1 ; line:151 col:22
  %tmp134 = call <4 x float> @"dx.hl.matldst.colLoad.<4 x float> (i32, %class.matrix.float.2.2*)"(i32 0, %class.matrix.float.2.2* %tmp133) ; line:151 col:22
  %tmp135 = call <4 x float> @"dx.hl.matldst.colStore.<4 x float> (i32, %class.matrix.float.2.2*, <4 x float>)"(i32 1, %class.matrix.float.2.2* %tmp132, <4 x float> %tmp134) ; line:151 col:22


  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.ConsumeStructuredBuffer<vector<float, 2> >"(i32 160, %"class.ConsumeStructuredBuffer<vector<float, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 36876, i32 8 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 -1)
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, i8 3, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec2:%.*]] = insertelement <2 x float> [[ping]], float [[val1]], i64 1
  %tmp136 = load %"class.ConsumeStructuredBuffer<vector<float, 2> >", %"class.ConsumeStructuredBuffer<vector<float, 2> >"* @"\01?CVecBuf@@3V?$ConsumeStructuredBuffer@V?$vector@M$01@@@@A" ; line:159 col:18
  %tmp137 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.ConsumeStructuredBuffer<vector<float, 2> >" %tmp136) ; line:159 col:18
  %tmp138 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %tmp137, %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.ConsumeStructuredBuffer<vector<float, 2> >" zeroinitializer) ; line:159 col:18
  %tmp139 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 281, %dx.types.Handle %tmp138) #0 ; line:159 col:18
  %tmp140 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp138, i32 %tmp139) #0 ; line:159 col:18
  %tmp141 = load <2 x float>, <2 x float>* %tmp140 ; line:159 col:18


  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.AppendStructuredBuffer<vector<float, 2> >"(i32 160, %"class.AppendStructuredBuffer<vector<float, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 36876, i32 8 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 1)
  ; CHECK: [[val0:%.*]] = extractelement <2 x float> [[vec2]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x float> [[vec2]], i64 1
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, float [[val0]], float [[val1]], float undef, float undef, i8 3, i32 4)
  %tmp142 = load %"class.AppendStructuredBuffer<vector<float, 2> >", %"class.AppendStructuredBuffer<vector<float, 2> >"* @"\01?AVecBuf@@3V?$AppendStructuredBuffer@V?$vector@M$01@@@@A" ; line:159 col:3
  %tmp143 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.AppendStructuredBuffer<vector<float, 2> >" %tmp142) ; line:159 col:3
  %tmp144 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %tmp143, %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.AppendStructuredBuffer<vector<float, 2> >" zeroinitializer) ; line:159 col:3
  %tmp145 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 282, %dx.types.Handle %tmp144) #0 ; line:159 col:3
  %tmp146 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp144, i32 %tmp145) #0 ; line:159 col:3
  store <2 x float> %tmp141, <2 x float>* %tmp146 ; line:159 col:3

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.ConsumeStructuredBuffer<float [2]>"(i32 160, %"class.ConsumeStructuredBuffer<float [2]>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 36876, i32 8 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 -1)
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, i8 1, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 4, i8 1, i32 4)
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0

  %tmp147 = load %"class.ConsumeStructuredBuffer<float [2]>", %"class.ConsumeStructuredBuffer<float [2]>"* @"\01?CArrBuf@@3V?$ConsumeStructuredBuffer@$$BY01M@@A" ; line:167 col:18
  %tmp148 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<float [2]>\22)"(i32 0, %"class.ConsumeStructuredBuffer<float [2]>" %tmp147) ; line:167 col:18
  %tmp149 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle %tmp148, %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.ConsumeStructuredBuffer<float [2]>" zeroinitializer) ; line:167 col:18
  %tmp150 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 281, %dx.types.Handle %tmp149) #0 ; line:167 col:18
  %tmp151 = call [2 x float]* @"dx.hl.subscript.[].rn.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp149, i32 %tmp150) #0 ; line:167 col:18
  %tmp152 = getelementptr inbounds [2 x float], [2 x float]* %tmp151, i32 0, i32 0 ; line:167 col:3
  %tmp153 = load float, float* %tmp152 ; line:167 col:3
  %tmp154 = getelementptr inbounds [2 x float], [2 x float]* %tmp151, i32 0, i32 1 ; line:167 col:3
  %tmp155 = load float, float* %tmp154 ; line:167 col:3

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.AppendStructuredBuffer<float [2]>"(i32 160, %"class.AppendStructuredBuffer<float [2]>"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 36876, i32 8 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 1)
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, float [[val0]], float undef, float undef, float undef, i8 1, i32 4)
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 4, float [[val1]], float undef, float undef, float undef, i8 1, i32 4)

  %tmp156 = load %"class.AppendStructuredBuffer<float [2]>", %"class.AppendStructuredBuffer<float [2]>"* @"\01?AArrBuf@@3V?$AppendStructuredBuffer@$$BY01M@@A" ; line:167 col:3
  %tmp157 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<float [2]>\22)"(i32 0, %"class.AppendStructuredBuffer<float [2]>" %tmp156) ; line:167 col:3
  %tmp158 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle %tmp157, %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.AppendStructuredBuffer<float [2]>" zeroinitializer) ; line:167 col:3
  %tmp159 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 282, %dx.types.Handle %tmp158) #0 ; line:167 col:3
  %tmp160 = call [2 x float]* @"dx.hl.subscript.[].rn.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp158, i32 %tmp159) #0 ; line:167 col:3
  %tmp161 = getelementptr inbounds [2 x float], [2 x float]* %tmp160, i32 0, i32 0 ; line:167 col:3
  store float %tmp153, float* %tmp161 ; line:167 col:3
  %tmp162 = getelementptr inbounds [2 x float], [2 x float]* %tmp160, i32 0, i32 1 ; line:167 col:3
  store float %tmp155, float* %tmp162 ; line:167 col:3

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.ConsumeStructuredBuffer<Vector<float, 2> >"(i32 160, %"class.ConsumeStructuredBuffer<Vector<float, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 37644, i32 32 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 -1)
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[vec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 16, i8 1, i32 8)
  ; CHECK: [[dval:%.*]] = extractvalue %dx.types.ResRet.f64 [[ld]], 0
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 24, i8 3, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[ping:%.*]] = insertelement <2 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec2:%.*]] = insertelement <2 x float> [[ping]], float [[val1]], i64 1

  %tmp163 = load %"class.ConsumeStructuredBuffer<Vector<float, 2> >", %"class.ConsumeStructuredBuffer<Vector<float, 2> >"* @"\01?CSVecBuf@@3V?$ConsumeStructuredBuffer@U?$Vector@M$01@@@@A" ; line:175 col:19
  %tmp164 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<Vector<float, 2> >\22)"(i32 0, %"class.ConsumeStructuredBuffer<Vector<float, 2> >" %tmp163) ; line:175 col:19
  %tmp165 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle %tmp164, %dx.types.ResourceProperties { i32 4876, i32 32 }, %"class.ConsumeStructuredBuffer<Vector<float, 2> >" zeroinitializer) ; line:175 col:19
  %tmp166 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 281, %dx.types.Handle %tmp165) #0 ; line:175 col:19
  %tmp167 = call %"struct.Vector<float, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp165, i32 %tmp166) #0 ; line:175 col:19
  %tmp168 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp167, i32 0, i32 0 ; line:175 col:3
  %tmp169 = load <4 x float>, <4 x float>* %tmp168 ; line:175 col:3
  %tmp170 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp167, i32 0, i32 1 ; line:175 col:3
  %tmp171 = load double, double* %tmp170 ; line:175 col:3
  %tmp172 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp167, i32 0, i32 2 ; line:175 col:3
  %tmp173 = load <2 x float>, <2 x float>* %tmp172 ; line:175 col:3


  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.AppendStructuredBuffer<Vector<float, 2> >"(i32 160, %"class.AppendStructuredBuffer<Vector<float, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 37644, i32 32 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 1)
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[vec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[vec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[vec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[vec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  ; CHECK: call void @dx.op.rawBufferStore.f64(i32 140, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 16, double [[dval]], double undef, double undef, double undef, i8 1, i32 8)
  ; CHECK: [[val0:%.*]] = extractelement <2 x float> [[vec2]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <2 x float> [[vec2]], i64 1
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 24, float [[val0]], float [[val1]], float undef, float undef, i8 3, i32 4)
  %tmp174 = load %"class.AppendStructuredBuffer<Vector<float, 2> >", %"class.AppendStructuredBuffer<Vector<float, 2> >"* @"\01?ASVecBuf@@3V?$AppendStructuredBuffer@U?$Vector@M$01@@@@A" ; line:175 col:3
  %tmp175 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<Vector<float, 2> >\22)"(i32 0, %"class.AppendStructuredBuffer<Vector<float, 2> >" %tmp174) ; line:175 col:3
  %tmp176 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle %tmp175, %dx.types.ResourceProperties { i32 4876, i32 32 }, %"class.AppendStructuredBuffer<Vector<float, 2> >" zeroinitializer) ; line:175 col:3
  %tmp177 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 282, %dx.types.Handle %tmp176) #0 ; line:175 col:3
  %tmp178 = call %"struct.Vector<float, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp176, i32 %tmp177) #0 ; line:175 col:3
  %tmp179 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp178, i32 0, i32 0 ; line:175 col:3
  store <4 x float> %tmp169, <4 x float>* %tmp179 ; line:175 col:3
  %tmp180 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp178, i32 0, i32 1 ; line:175 col:3
  store double %tmp171, double* %tmp180 ; line:175 col:3
  %tmp181 = getelementptr inbounds %"struct.Vector<float, 2>", %"struct.Vector<float, 2>"* %tmp178, i32 0, i32 2 ; line:175 col:3
  store <2 x float> %tmp173, <2 x float>* %tmp181 ; line:175 col:3


  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.ConsumeStructuredBuffer<matrix<float, 2, 2> >"(i32 160, %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 37388, i32 16 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 -1)
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[vec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  ; CHECK: [[rvec4:%.*]] = shufflevector <4 x float> [[vec4]], <4 x float> [[vec4]], <4 x i32> <i32 0, i32 2, i32 1, i32 3>
  %tmp182 = load %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >", %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >"* @"\01?CMatBuf@@3V?$ConsumeStructuredBuffer@V?$matrix@M$01$01@@@@A" ; line:183 col:18
  %tmp183 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 0, %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >" %tmp182) ; line:183 col:18
  %tmp184 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle %tmp183, %dx.types.ResourceProperties { i32 4620, i32 16 }, %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >" zeroinitializer) ; line:183 col:18
  %tmp185 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 281, %dx.types.Handle %tmp184) #0 ; line:183 col:18
  %tmp186 = call %class.matrix.float.2.2* @"dx.hl.subscript.[].rn.%class.matrix.float.2.2* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp184, i32 %tmp185) #0 ; line:183 col:18
  %tmp187 = call <4 x float> @"dx.hl.matldst.colLoad.<4 x float> (i32, %class.matrix.float.2.2*)"(i32 0, %class.matrix.float.2.2* %tmp186) ; line:183 col:18
  %col2row10 = shufflevector <4 x float> %tmp187, <4 x float> %tmp187, <4 x i32> <i32 0, i32 2, i32 1, i32 3> ; line:183 col:18

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.AppendStructuredBuffer<matrix<float, 2, 2> >"(i32 160, %"class.AppendStructuredBuffer<matrix<float, 2, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 37388, i32 16 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 1)
  ; CHECK: [[cvec4:%.*]] = shufflevector <4 x float> [[rvec4]], <4 x float> [[rvec4]], <4 x i32> <i32 0, i32 2, i32 1, i32 3>
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[cvec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[cvec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[cvec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[cvec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, float [[val0]], float [[val1]], float [[val2]], float [[val3]]

  %tmp188 = load %"class.AppendStructuredBuffer<matrix<float, 2, 2> >", %"class.AppendStructuredBuffer<matrix<float, 2, 2> >"* @"\01?AMatBuf@@3V?$AppendStructuredBuffer@V?$matrix@M$01$01@@@@A" ; line:183 col:3
  %tmp189 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 0, %"class.AppendStructuredBuffer<matrix<float, 2, 2> >" %tmp188) ; line:183 col:3
  %tmp190 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle %tmp189, %dx.types.ResourceProperties { i32 4620, i32 16 }, %"class.AppendStructuredBuffer<matrix<float, 2, 2> >" zeroinitializer) ; line:183 col:3
  %tmp191 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 282, %dx.types.Handle %tmp190) #0 ; line:183 col:3
  %tmp192 = call %class.matrix.float.2.2* @"dx.hl.subscript.[].rn.%class.matrix.float.2.2* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp190, i32 %tmp191) #0 ; line:183 col:3
  %row2col11 = shufflevector <4 x float> %col2row10, <4 x float> %col2row10, <4 x i32> <i32 0, i32 2, i32 1, i32 3> ; line:183 col:3
  call void @"dx.hl.matldst.colStore.void (i32, %class.matrix.float.2.2*, <4 x float>)"(i32 1, %class.matrix.float.2.2* %tmp192, <4 x float> %row2col11) ; line:183 col:3


  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >"(i32 160, %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 37388, i32 32 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 -1)
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[vec4:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 16, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> undef, float [[val0]], i64 0
  ; CHECK: [[pong:%.*]] = insertelement <4 x float> [[ping]], float [[val1]], i64 1
  ; CHECK: [[ping:%.*]] = insertelement <4 x float> [[pong]], float [[val2]], i64 2
  ; CHECK: [[mat:%.*]] = insertelement <4 x float> [[ping]], float [[val3]], i64 3
  %tmp193 = load %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >", %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >"* @"\01?CSMatBuf@@3V?$ConsumeStructuredBuffer@U?$Matrix@M$01$01@@@@A" ; line:191 col:19
  %tmp194 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 0, %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >" %tmp193) ; line:191 col:19
  %tmp195 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle %tmp194, %dx.types.ResourceProperties { i32 4620, i32 32 }, %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >" zeroinitializer) ; line:191 col:19
  %tmp196 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 281, %dx.types.Handle %tmp195) #0 ; line:191 col:19
  %tmp197 = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp195, i32 %tmp196) #0 ; line:191 col:19
  %tmp198 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp197, i32 0, i32 0 ; line:191 col:3
  %tmp199 = load <4 x float>, <4 x float>* %tmp198 ; line:191 col:3
  %tmp200 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp197, i32 0, i32 1 ; line:191 col:3
  %tmp201 = call <4 x float> @"dx.hl.matldst.colLoad.<4 x float> (i32, %class.matrix.float.2.2*)"(i32 0, %class.matrix.float.2.2* %tmp200) ; line:191 col:3

  ; CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.AppendStructuredBuffer<Matrix<float, 2, 2> >"(i32 160, %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >"
  ; CHECK: [[anhdl:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 37388, i32 32 })
  ; CHECK: [[ct:%.*]] = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle [[anhdl]], i8 1)
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[vec4]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[vec4]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[vec4]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[vec4]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 0, float [[val0]], float [[val1]], float [[val2]], float [[val3]]
  ; CHECK: [[val0:%.*]] = extractelement <4 x float> [[mat]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <4 x float> [[mat]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <4 x float> [[mat]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <4 x float> [[mat]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle [[anhdl]], i32 [[ct]], i32 16, float [[val0]], float [[val1]], float [[val2]], float [[val3]]

  %tmp202 = load %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >", %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >"* @"\01?ASMatBuf@@3V?$AppendStructuredBuffer@U?$Matrix@M$01$01@@@@A" ; line:191 col:3
  %tmp203 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 0, %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >" %tmp202) ; line:191 col:3
  %tmp204 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle %tmp203, %dx.types.ResourceProperties { i32 4620, i32 32 }, %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >" zeroinitializer) ; line:191 col:3
  %tmp205 = call i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32 282, %dx.types.Handle %tmp204) #0 ; line:191 col:3
  %tmp206 = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %tmp204, i32 %tmp205) #0 ; line:191 col:3
  %tmp207 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp206, i32 0, i32 0 ; line:191 col:3
  store <4 x float> %tmp199, <4 x float>* %tmp207 ; line:191 col:3
  %tmp208 = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* %tmp206, i32 0, i32 1 ; line:191 col:3
  %tmp209 = call <4 x float> @"dx.hl.matldst.colStore.<4 x float> (i32, %class.matrix.float.2.2*, <4 x float>)"(i32 1, %class.matrix.float.2.2* %tmp208, <4 x float> %tmp201) ; line:191 col:3


  ; CHECK: ret void
  ret void ; line:193 col:1
}

declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <2 x i1>)"(i32, %dx.types.Handle, i32, <2 x i1>) #0
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32, %struct.RWByteAddressBuffer) #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer) #1
declare <2 x i1> @"dx.hl.op.ro.<2 x i1> (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2
declare [2 x float]* @"dx.hl.op.ro.[2 x float]* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2
declare %"struct.Vector<float, 2>"* @"dx.hl.op.ro.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2
declare %"struct.Matrix<float, 2, 2>"* @"dx.hl.op.ro.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2
declare <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32, %"class.RWStructuredBuffer<vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<vector<float, 2> >") #1
declare [2 x float]* @"dx.hl.subscript.[].rn.[2 x float]* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<float [2]>\22)"(i32, %"class.RWStructuredBuffer<float [2]>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<float [2]>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<float [2]>") #1
declare %"struct.Vector<float, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32, %"class.RWStructuredBuffer<Vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<Vector<float, 2> >") #1
declare %class.matrix.float.2.2* @"dx.hl.subscript.[].rn.%class.matrix.float.2.2* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32, %"class.RWStructuredBuffer<matrix<float, 2, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<matrix<float, 2, 2> >") #1
declare %"struct.Matrix<float, 2, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >") #1
declare i32 @"dx.hl.op..i32 (i32, %dx.types.Handle)"(i32, %dx.types.Handle) #0
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<vector<float, 2> >\22)"(i32, %"class.AppendStructuredBuffer<vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.AppendStructuredBuffer<vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<vector<float, 2> >\22)"(i32, %"class.ConsumeStructuredBuffer<vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.ConsumeStructuredBuffer<vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<float [2]>\22)"(i32, %"class.AppendStructuredBuffer<float [2]>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<float [2]>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.AppendStructuredBuffer<float [2]>") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<float [2]>\22)"(i32, %"class.ConsumeStructuredBuffer<float [2]>") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<float [2]>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.ConsumeStructuredBuffer<float [2]>") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<Vector<float, 2> >\22)"(i32, %"class.AppendStructuredBuffer<Vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<Vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.AppendStructuredBuffer<Vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<Vector<float, 2> >\22)"(i32, %"class.ConsumeStructuredBuffer<Vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<Vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.ConsumeStructuredBuffer<Vector<float, 2> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<matrix<float, 2, 2> >\22)"(i32, %"class.AppendStructuredBuffer<matrix<float, 2, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<matrix<float, 2, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.AppendStructuredBuffer<matrix<float, 2, 2> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<matrix<float, 2, 2> >\22)"(i32, %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<matrix<float, 2, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32, %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >") #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32, %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >") #1
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >") #1
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32, %dx.types.Handle, i32, float) #0
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <4 x float>)"(i32, %dx.types.Handle, i32, <4 x float>) #0
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, double)"(i32, %dx.types.Handle, i32, double) #0
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <2 x float>)"(i32, %dx.types.Handle, i32, <2 x float>) #0
declare <4 x float> @"dx.hl.op.ro.<4 x float> (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2
declare <4 x float> @"dx.hl.matldst.colLoad.<4 x float> (i32, %class.matrix.float.2.2*)"(i32, %class.matrix.float.2.2*) #2
declare <4 x float> @"dx.hl.matldst.colStore.<4 x float> (i32, %class.matrix.float.2.2*, <4 x float>)"(i32, %class.matrix.float.2.2*, <4 x float>) #0
declare void @"dx.hl.matldst.colStore.void (i32, %class.matrix.float.2.2*, <4 x float>)"(i32, %class.matrix.float.2.2*, <4 x float>) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !43}
!dx.entryPoints = !{!50}
!dx.fnprops = !{!72}
!dx.options = !{!73, !74}

!3 = !{i32 1, i32 6}
!4 = !{i32 1, i32 9}
!5 = !{!"vs", i32 6, i32 6}
!6 = !{i32 0, %"class.RWStructuredBuffer<vector<float, 2> >" undef, !7, %"class.RWStructuredBuffer<float [2]>" undef, !12, %"class.RWStructuredBuffer<Vector<float, 2> >" undef, !16, %"struct.Vector<float, 2>" undef, !21, %"class.RWStructuredBuffer<matrix<float, 2, 2> >" undef, !29, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >" undef, !35, %"struct.Matrix<float, 2, 2>" undef, !39, %"class.ConsumeStructuredBuffer<vector<float, 2> >" undef, !7, %"class.ConsumeStructuredBuffer<float [2]>" undef, !12, %"class.ConsumeStructuredBuffer<Vector<float, 2> >" undef, !16, %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >" undef, !29, %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >" undef, !35, %"class.AppendStructuredBuffer<vector<float, 2> >" undef, !7, %"class.AppendStructuredBuffer<float [2]>" undef, !12, %"class.AppendStructuredBuffer<Vector<float, 2> >" undef, !16, %"class.AppendStructuredBuffer<matrix<float, 2, 2> >" undef, !29, %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >" undef, !35}
!7 = !{i32 8, !8, !9}
!8 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9}
!9 = !{i32 0, !10}
!10 = !{!11}
!11 = !{i32 0, <2 x float> undef}
!12 = !{i32 20, !8, !13}
!13 = !{i32 0, !14}
!14 = !{!15}
!15 = !{i32 0, [2 x float] undef}
!16 = !{i32 32, !17, !18}
!17 = !{i32 6, !"h", i32 3, i32 0}
!18 = !{i32 0, !19}
!19 = !{!20}
!20 = !{i32 0, %"struct.Vector<float, 2>" undef}
!21 = !{i32 32, !22, !23, !24, !25}
!22 = !{i32 6, !"pad1", i32 3, i32 0, i32 7, i32 9}
!23 = !{i32 6, !"pad2", i32 3, i32 16, i32 7, i32 10}
!24 = !{i32 6, !"v", i32 3, i32 24, i32 7, i32 9}
!25 = !{i32 0, !26}
!26 = !{!27, !28}
!27 = !{i32 0, float undef}
!28 = !{i32 1, i64 2}
!29 = !{i32 24, !30, !32}
!30 = !{i32 6, !"h", i32 2, !31, i32 3, i32 0, i32 7, i32 9}
!31 = !{i32 2, i32 2, i32 2}
!32 = !{i32 0, !33}
!33 = !{!34}
!34 = !{i32 0, %class.matrix.float.2.2 undef}
!35 = !{i32 40, !17, !36}
!36 = !{i32 0, !37}
!37 = !{!38}
!38 = !{i32 0, %"struct.Matrix<float, 2, 2>" undef}
!39 = !{i32 40, !22, !40, !41}
!40 = !{i32 6, !"m", i32 2, !31, i32 3, i32 16, i32 7, i32 9}
!41 = !{i32 0, !42}
!42 = !{!27, !28, !28}
!43 = !{i32 1, void (i32)* @main, !44}
!44 = !{!45, !47}
!45 = !{i32 1, !46, !46}
!46 = !{}
!47 = !{i32 0, !48, !49}
!48 = !{i32 4, !"IX0", i32 7, i32 5}
!49 = !{i32 0}
!50 = !{void (i32)* @main, !"main", null, !51, null}
!51 = !{null, !52, null, null}
!52 = !{!53, !54, !56, !57, !59, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71}
!53 = !{i32 0, %struct.RWByteAddressBuffer* @"\01?BabBuf@@3URWByteAddressBuffer@@A", !"BabBuf", i32 0, i32 1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!54 = !{i32 1, %"class.RWStructuredBuffer<vector<float, 2> >"* @"\01?VecBuf@@3V?$RWStructuredBuffer@V?$vector@M$01@@@@A", !"VecBuf", i32 0, i32 2, i32 1, i32 12, i1 false, i1 false, i1 false, !55}
!55 = !{i32 1, i32 8}
!56 = !{i32 2, %"class.RWStructuredBuffer<float [2]>"* @"\01?ArrBuf@@3V?$RWStructuredBuffer@$$BY01M@@A", !"ArrBuf", i32 0, i32 3, i32 1, i32 12, i1 false, i1 false, i1 false, !55}
!57 = !{i32 3, %"class.RWStructuredBuffer<Vector<float, 2> >"* @"\01?SVecBuf@@3V?$RWStructuredBuffer@U?$Vector@M$01@@@@A", !"SVecBuf", i32 0, i32 4, i32 1, i32 12, i1 false, i1 false, i1 false, !58}
!58 = !{i32 1, i32 32}
!59 = !{i32 4, %"class.RWStructuredBuffer<matrix<float, 2, 2> >"* @"\01?MatBuf@@3V?$RWStructuredBuffer@V?$matrix@M$01$01@@@@A", !"MatBuf", i32 0, i32 5, i32 1, i32 12, i1 false, i1 false, i1 false, !60}
!60 = !{i32 1, i32 16}
!61 = !{i32 5, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >"* @"\01?SMatBuf@@3V?$RWStructuredBuffer@U?$Matrix@M$01$01@@@@A", !"SMatBuf", i32 0, i32 6, i32 1, i32 12, i1 false, i1 false, i1 false, !58}
!62 = !{i32 6, %"class.ConsumeStructuredBuffer<vector<float, 2> >"* @"\01?CVecBuf@@3V?$ConsumeStructuredBuffer@V?$vector@M$01@@@@A", !"CVecBuf", i32 0, i32 7, i32 1, i32 12, i1 false, i1 false, i1 false, !55}
!63 = !{i32 7, %"class.ConsumeStructuredBuffer<float [2]>"* @"\01?CArrBuf@@3V?$ConsumeStructuredBuffer@$$BY01M@@A", !"CArrBuf", i32 0, i32 8, i32 1, i32 12, i1 false, i1 false, i1 false, !55}
!64 = !{i32 8, %"class.ConsumeStructuredBuffer<Vector<float, 2> >"* @"\01?CSVecBuf@@3V?$ConsumeStructuredBuffer@U?$Vector@M$01@@@@A", !"CSVecBuf", i32 0, i32 9, i32 1, i32 12, i1 false, i1 false, i1 false, !58}
!65 = !{i32 9, %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >"* @"\01?CMatBuf@@3V?$ConsumeStructuredBuffer@V?$matrix@M$01$01@@@@A", !"CMatBuf", i32 0, i32 10, i32 1, i32 12, i1 false, i1 false, i1 false, !60}
!66 = !{i32 10, %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >"* @"\01?CSMatBuf@@3V?$ConsumeStructuredBuffer@U?$Matrix@M$01$01@@@@A", !"CSMatBuf", i32 0, i32 11, i32 1, i32 12, i1 false, i1 false, i1 false, !58}
!67 = !{i32 11, %"class.AppendStructuredBuffer<vector<float, 2> >"* @"\01?AVecBuf@@3V?$AppendStructuredBuffer@V?$vector@M$01@@@@A", !"AVecBuf", i32 0, i32 12, i32 1, i32 12, i1 false, i1 false, i1 false, !55}
!68 = !{i32 12, %"class.AppendStructuredBuffer<float [2]>"* @"\01?AArrBuf@@3V?$AppendStructuredBuffer@$$BY01M@@A", !"AArrBuf", i32 0, i32 13, i32 1, i32 12, i1 false, i1 false, i1 false, !55}
!69 = !{i32 13, %"class.AppendStructuredBuffer<Vector<float, 2> >"* @"\01?ASVecBuf@@3V?$AppendStructuredBuffer@U?$Vector@M$01@@@@A", !"ASVecBuf", i32 0, i32 14, i32 1, i32 12, i1 false, i1 false, i1 false, !58}
!70 = !{i32 14, %"class.AppendStructuredBuffer<matrix<float, 2, 2> >"* @"\01?AMatBuf@@3V?$AppendStructuredBuffer@V?$matrix@M$01$01@@@@A", !"AMatBuf", i32 0, i32 15, i32 1, i32 12, i1 false, i1 false, i1 false, !60}
!71 = !{i32 15, %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >"* @"\01?ASMatBuf@@3V?$AppendStructuredBuffer@U?$Matrix@M$01$01@@@@A", !"ASMatBuf", i32 0, i32 16, i32 1, i32 12, i1 false, i1 false, i1 false, !58}
!72 = !{void (i32)* @main, i32 1}
!73 = !{i32 64}
!74 = !{i32 -1}
