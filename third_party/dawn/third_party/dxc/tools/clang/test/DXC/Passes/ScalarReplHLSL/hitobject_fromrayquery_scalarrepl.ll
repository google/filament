; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; COM: Original HLSL code
; COM: RaytracingAccelerationStructure RTAS;
; COM: RWStructuredBuffer<float> UAV : register(u0);
; COM: RWByteAddressBuffer inbuf;
; COM: RWByteAddressBuffer outbuf;
; COM: 
; COM: RayDesc MakeRayDesc() {
; COM:   RayDesc desc;
; COM:   desc.Origin = float3(0, 0, 0);
; COM:   desc.Direction = float3(1, 0, 0);
; COM:   desc.TMin = 0.0f;
; COM:   desc.TMax = 9999.0;
; COM:   return desc;
; COM: }
; COM: 
; COM: struct CustomAttrs {
; COM:   float x;
; COM:   float y;
; COM: };
; COM: 
; COM: void Use(in dx::HitObject hit) {
; COM:   dx::MaybeReorderThread(hit);
; COM: }
; COM: 
; COM: [shader("raygeneration")]
; COM: void main() {
; COM:   RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
; COM:   RayDesc ray = MakeRayDesc();
; COM:   q.TraceRayInline(RTAS, RAY_FLAG_NONE, 0xFF, ray);
; COM: 
; COM:   Use(dx::HitObject::FromRayQuery(q));
; COM: 
; COM:   CustomAttrs attrs;
; COM:   attrs.x = inbuf.Load(0);
; COM:   attrs.y = inbuf.Load(4);
; COM:   Use(dx::HitObject::FromRayQuery(q, 16, attrs));
; COM: 
; COM:   attrs.x = inbuf.Load(8);
; COM:   attrs.y = inbuf.Load(12);
; COM:   Use(dx::HitObject::FromRayQuery(q, 17, attrs));
; COM: 
; COM:   outbuf.Store(0, attrs.x);
; COM:   outbuf.Store(4, attrs.y);
; COM: }

;
; Buffer Definitions:
;
; cbuffer $Globals
; {
;
;   [0 x i8] (type annotation not present)
;
; }
;
; Resource bind info for UAV
; {
;
;   float $Element;                                   ; Offset:    0 Size:     4
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; $Globals                          cbuffer      NA          NA     CB0   cb4294967295     1
; RTAS                              texture     i32         ras      T0t4294967295,space4294967295     1
; UAV                                   UAV  struct         r/w      U0             u0     1
; inbuf                                 UAV    byte         r/w      U1u4294967295,space4294967295     1
; outbuf                                UAV    byte         r/w      U2u4294967295,space4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%"class.RWStructuredBuffer<float>" = type { float }
%struct.RWByteAddressBuffer = type { i32 }
%ConstantBuffer = type opaque
%"class.RayQuery<5, 0>" = type { i32 }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%dx.types.HitObject = type { i8* }
%struct.CustomAttrs = type { float, float }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.dx::HitObject" = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4
@"\01?UAV@@3V?$RWStructuredBuffer@M@@A" = external global %"class.RWStructuredBuffer<float>", align 4
@"\01?inbuf@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4
@"\01?outbuf@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4
@"$Globals" = external constant %ConstantBuffer

; CHECK: %[[RQA:[^ ]+]] = alloca i32
; CHECK: %[[XATTRA:[^ ]+]] = alloca float
; CHECK: %[[YATTRA:[^ ]+]] = alloca float
; CHECK: %[[ATTRA0:[^ ]+]] = alloca %struct.CustomAttrs
; CHECK: %[[ATTRA1:[^ ]+]] = alloca %struct.CustomAttrs

; COM: Check same query handle used for TraceRayInline and the FromRayQuery calls
; CHECK: %[[RQH:[^ ]+]] = load i32, i32* %[[RQA]]
; CHECK: call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %[[RQH]],
    
; COM: Check RQ handle loaded for first FromRayQuery call
; CHECK: %[[RQH0:[^ ]+]] = load i32, i32* %[[RQA]]
; CHECK: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32)"(i32 363, %dx.types.HitObject* %{{[^ ]+}}, i32 %[[RQH0]])

; COM: Check buffer loads for first FromRayQuery-with-attrs call
; CHECK: %[[XI0:[^ ]+]] = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %{{[^ ]+}}, i32 0)
; CHECK: %[[XF0:[^ ]+]] = uitofp i32 %[[XI0]] to float
; CHECK: store float %[[XF0]], float* %[[XATTRA]], align 4
; CHECK: %[[YI0:[^ ]+]] = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %{{[^ ]+}}, i32 4)
; CHECK: %[[YF0:[^ ]+]] = uitofp i32 %[[YI0]] to float
; CHECK: store float %[[YF0]], float* %[[YATTRA]], align 4

; COM: Check that values from buffer flow into first FromRayQuery-with-attrs call
; CHECK: %[[XPTR0:[^ ]+]] = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %[[ATTRA0]], i32 0, i32 0
; CHECK: %[[XF1:[^ ]+]] = load float, float* %[[XATTRA]]
; CHECK: store float %[[XF1]], float* %[[XPTR0]]
; CHECK: %[[YPTR0:[^ ]+]] = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %[[ATTRA0]], i32 0, i32 1
; CHECK: %[[YF1:[^ ]+]] = load float, float* %[[YATTRA]]
; CHECK: store float %[[YF1]], float* %[[YPTR0]]
; CHECK: %[[RQH1:[^ ]+]] = load i32, i32* %[[RQA]]
; CHECK: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32, %struct.CustomAttrs*)"(i32 363, %dx.types.HitObject* %{{[^ ]+}}, i32 %[[RQH1]], i32 16, %struct.CustomAttrs* %[[ATTRA0]])

; COM: Check buffer loads for second FromRayQuery-with-attrs call
; CHECK: %[[XI1:[^ ]+]] = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %{{[^ ]+}}, i32 8)
; CHECK: %[[XF1:[^ ]+]] = uitofp i32 %[[XI1]] to float
; CHECK: store float %[[XF1]], float* %[[XATTRA]], align 4
; CHECK: %[[YI1:[^ ]+]] = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %{{[^ ]+}}, i32 12)
; CHECK: %[[YF1:[^ ]+]] = uitofp i32 %[[YI1]] to float
; CHECK: store float %[[YF1]], float* %[[YATTRA]], align 4

; COM: Check that values from buffer flow into second FromRayQuery-with-attrs call
; CHECK: %[[XPTR1:[^ ]+]] = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %[[ATTRA1]], i32 0, i32 0
; CHECK: %[[XF2:[^ ]+]] = load float, float* %[[XATTRA]]
; CHECK: store float %[[XF2]], float* %[[XPTR1]]
; CHECK: %[[YPTR1:[^ ]+]] = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %[[ATTRA1]], i32 0, i32 1
; CHECK: %[[YF2:[^ ]+]] = load float, float* %[[YATTRA]]
; CHECK: store float %[[YF2]], float* %[[YPTR1]]
; CHECK: %[[RQH2:[^ ]+]] = load i32, i32* %[[RQA]]
; CHECK: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32, %struct.CustomAttrs*)"(i32 363, %dx.types.HitObject* %{{[^ ]+}}, i32 %[[RQH2]], i32 17, %struct.CustomAttrs* %[[ATTRA1]])


; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
entry:
  %q = alloca %"class.RayQuery<5, 0>", align 4
  %ray = alloca %struct.RayDesc, align 4
  %agg.tmp = alloca %dx.types.HitObject, align 4
  %attrs = alloca %struct.CustomAttrs, align 4
  %agg.tmp4 = alloca %dx.types.HitObject, align 4
  %agg.tmp11 = alloca %dx.types.HitObject, align 4
  %0 = bitcast %"class.RayQuery<5, 0>"* %q to i8*, !dbg !45 ; line:26 col:3
  call void @llvm.lifetime.start(i64 4, i8* %0) #0, !dbg !45 ; line:26 col:3
  %q14 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 5, i32 0), !dbg !49 ; line:26 col:78
  %1 = getelementptr inbounds %"class.RayQuery<5, 0>", %"class.RayQuery<5, 0>"* %q, i32 0, i32 0, !dbg !49 ; line:26 col:78
  store i32 %q14, i32* %1, !dbg !49 ; line:26 col:78
  %2 = bitcast %struct.RayDesc* %ray to i8*, !dbg !50 ; line:27 col:3
  call void @llvm.lifetime.start(i64 32, i8* %2) #0, !dbg !50 ; line:27 col:3
  %Origin.i = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %ray, i32 0, i32 0, !dbg !51 ; line:8 col:8
  store <3 x float> zeroinitializer, <3 x float>* %Origin.i, align 4, !dbg !54, !tbaa !55, !alias.scope !58 ; line:8 col:15
  %Direction.i = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %ray, i32 0, i32 2, !dbg !61 ; line:9 col:8
  store <3 x float> <float 1.000000e+00, float 0.000000e+00, float 0.000000e+00>, <3 x float>* %Direction.i, align 4, !dbg !62, !tbaa !55, !alias.scope !58 ; line:9 col:18
  %TMin.i = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %ray, i32 0, i32 1, !dbg !63 ; line:10 col:8
  store float 0.000000e+00, float* %TMin.i, align 4, !dbg !64, !tbaa !65, !alias.scope !58 ; line:10 col:13
  %TMax.i = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %ray, i32 0, i32 3, !dbg !67 ; line:11 col:8
  store float 9.999000e+03, float* %TMax.i, align 4, !dbg !68, !tbaa !65, !alias.scope !58 ; line:11 col:13
  %3 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !69 ; line:28 col:3
  %4 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %3), !dbg !69 ; line:28 col:3
  %5 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %4, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef), !dbg !69 ; line:28 col:3
  call void @"dx.hl.op..void (i32, %\22class.RayQuery<5, 0>\22*, %dx.types.Handle, i32, i32, %struct.RayDesc*)"(i32 325, %"class.RayQuery<5, 0>"* %q, %dx.types.Handle %5, i32 0, i32 255, %struct.RayDesc* %ray), !dbg !69 ; line:28 col:3
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %\22class.RayQuery<5, 0>\22*)"(i32 363, %dx.types.HitObject* %agg.tmp, %"class.RayQuery<5, 0>"* %q), !dbg !70 ; line:30 col:7
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 359, %dx.types.HitObject* %agg.tmp) #0, !dbg !71 ; line:21 col:3
  %6 = bitcast %struct.CustomAttrs* %attrs to i8*, !dbg !74 ; line:32 col:3
  call void @llvm.lifetime.start(i64 8, i8* %6) #0, !dbg !74 ; line:32 col:3
  %7 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?inbuf@@3URWByteAddressBuffer@@A", !dbg !75 ; line:33 col:13
  %8 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %7), !dbg !75 ; line:33 col:13
  %9 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %8, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef), !dbg !75 ; line:33 col:13
  %10 = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %9, i32 0), !dbg !75 ; line:33 col:13
  %conv = uitofp i32 %10 to float, !dbg !75 ; line:33 col:13
  %x = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 0, !dbg !76 ; line:33 col:9
  store float %conv, float* %x, align 4, !dbg !77, !tbaa !65 ; line:33 col:11
  %11 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?inbuf@@3URWByteAddressBuffer@@A", !dbg !78 ; line:34 col:13
  %12 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %11), !dbg !78 ; line:34 col:13
  %13 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %12, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef), !dbg !78 ; line:34 col:13
  %14 = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %13, i32 4), !dbg !78 ; line:34 col:13
  %conv3 = uitofp i32 %14 to float, !dbg !78 ; line:34 col:13
  %y = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 1, !dbg !79 ; line:34 col:9
  store float %conv3, float* %y, align 4, !dbg !80, !tbaa !65 ; line:34 col:11
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %\22class.RayQuery<5, 0>\22*, i32, %struct.CustomAttrs*)"(i32 363, %dx.types.HitObject* %agg.tmp4, %"class.RayQuery<5, 0>"* %q, i32 16, %struct.CustomAttrs* %attrs), !dbg !81 ; line:35 col:7
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 359, %dx.types.HitObject* %agg.tmp4) #0, !dbg !82 ; line:21 col:3
  %15 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?inbuf@@3URWByteAddressBuffer@@A", !dbg !84 ; line:37 col:13
  %16 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %15), !dbg !84 ; line:37 col:13
  %17 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %16, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef), !dbg !84 ; line:37 col:13
  %18 = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %17, i32 8), !dbg !84 ; line:37 col:13
  %conv6 = uitofp i32 %18 to float, !dbg !84 ; line:37 col:13
  %x7 = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 0, !dbg !85 ; line:37 col:9
  store float %conv6, float* %x7, align 4, !dbg !86, !tbaa !65 ; line:37 col:11
  %19 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?inbuf@@3URWByteAddressBuffer@@A", !dbg !87 ; line:38 col:13
  %20 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %19), !dbg !87 ; line:38 col:13
  %21 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %20, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef), !dbg !87 ; line:38 col:13
  %22 = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %21, i32 12), !dbg !87 ; line:38 col:13
  %conv9 = uitofp i32 %22 to float, !dbg !87 ; line:38 col:13
  %y10 = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 1, !dbg !88 ; line:38 col:9
  store float %conv9, float* %y10, align 4, !dbg !89, !tbaa !65 ; line:38 col:11
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %\22class.RayQuery<5, 0>\22*, i32, %struct.CustomAttrs*)"(i32 363, %dx.types.HitObject* %agg.tmp11, %"class.RayQuery<5, 0>"* %q, i32 17, %struct.CustomAttrs* %attrs), !dbg !90 ; line:39 col:7
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 359, %dx.types.HitObject* %agg.tmp11) #0, !dbg !91 ; line:21 col:3
  %x12 = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 0, !dbg !93 ; line:41 col:25
  %23 = load float, float* %x12, align 4, !dbg !93, !tbaa !65 ; line:41 col:25
  %24 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !dbg !94 ; line:41 col:3
  %25 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %24), !dbg !94 ; line:41 col:3
  %26 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %25, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef), !dbg !94 ; line:41 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32 277, %dx.types.Handle %26, i32 0, float %23), !dbg !94 ; line:41 col:3
  %y13 = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 1, !dbg !95 ; line:42 col:25
  %27 = load float, float* %y13, align 4, !dbg !95, !tbaa !65 ; line:42 col:25
  %28 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !dbg !96 ; line:42 col:3
  %29 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %28), !dbg !96 ; line:42 col:3
  %30 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %29, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef), !dbg !96 ; line:42 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32 277, %dx.types.Handle %30, i32 4, float %27), !dbg !96 ; line:42 col:3
  %31 = bitcast %struct.CustomAttrs* %attrs to i8*, !dbg !97 ; line:43 col:1
  call void @llvm.lifetime.end(i64 8, i8* %31) #0, !dbg !97 ; line:43 col:1
  %32 = bitcast %struct.RayDesc* %ray to i8*, !dbg !97 ; line:43 col:1
  call void @llvm.lifetime.end(i64 32, i8* %32) #0, !dbg !97 ; line:43 col:1
  %33 = bitcast %"class.RayQuery<5, 0>"* %q to i8*, !dbg !97 ; line:43 col:1
  call void @llvm.lifetime.end(i64 4, i8* %33) #0, !dbg !97 ; line:43 col:1
  ret void, !dbg !97 ; line:43 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %\22class.RayQuery<5, 0>\22*, %dx.types.Handle, i32, i32, %struct.RayDesc*)"(i32, %"class.RayQuery<5, 0>"*, %dx.types.Handle, i32, i32, %struct.RayDesc*) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, %\22class.RayQuery<5, 0>\22*)"(i32, %dx.types.HitObject*, %"class.RayQuery<5, 0>"*) #0

; Function Attrs: nounwind readonly
declare i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, %\22class.RayQuery<5, 0>\22*, i32, %struct.CustomAttrs*)"(i32, %dx.types.HitObject*, %"class.RayQuery<5, 0>"*, i32, %struct.CustomAttrs*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32, %dx.types.Handle, i32, float) #0

; Function Attrs: nounwind
declare i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32, i32, i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.typeAnnotations = !{!4, !26}
!dx.entryPoints = !{!30}
!dx.fnprops = !{!42}
!dx.options = !{!43, !44}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{i32 1, i32 9}
!3 = !{!"lib", i32 6, i32 9}
!4 = !{i32 0, %"class.RWStructuredBuffer<float>" undef, !5, %struct.RayDesc undef, !10, %"class.dx::HitObject" undef, !15, %"class.RayQuery<5, 0>" undef, !17, %struct.CustomAttrs undef, !23}
!5 = !{i32 4, !6, !7}
!6 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9}
!7 = !{i32 0, !8}
!8 = !{!9}
!9 = !{i32 0, float undef}
!10 = !{i32 32, !11, !12, !13, !14}
!11 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9, i32 13, i32 3}
!12 = !{i32 6, !"TMin", i32 3, i32 12, i32 7, i32 9}
!13 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9, i32 13, i32 3}
!14 = !{i32 6, !"TMax", i32 3, i32 28, i32 7, i32 9}
!15 = !{i32 4, !16}
!16 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!17 = !{i32 4, !18, !19}
!18 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 5}
!19 = !{i32 0, !20}
!20 = !{!21, !22}
!21 = !{i32 1, i64 5}
!22 = !{i32 1, i64 0}
!23 = !{i32 8, !24, !25}
!24 = !{i32 6, !"x", i32 3, i32 0, i32 7, i32 9}
!25 = !{i32 6, !"y", i32 3, i32 4, i32 7, i32 9}
!26 = !{i32 1, void ()* @"\01?main@@YAXXZ", !27}
!27 = !{!28}
!28 = !{i32 1, !29, !29}
!29 = !{}
!30 = !{null, !"", null, !31, null}
!31 = !{!32, !35, !40, null}
!32 = !{!33}
!33 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !34}
!34 = !{i32 0, i32 4}
!35 = !{!36, !38, !39}
!36 = !{i32 0, %"class.RWStructuredBuffer<float>"* @"\01?UAV@@3V?$RWStructuredBuffer@M@@A", !"UAV", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !37}
!37 = !{i32 1, i32 4}
!38 = !{i32 1, %struct.RWByteAddressBuffer* @"\01?inbuf@@3URWByteAddressBuffer@@A", !"inbuf", i32 -1, i32 -1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!39 = !{i32 2, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !"outbuf", i32 -1, i32 -1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!40 = !{!41}
!41 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!42 = !{void ()* @"\01?main@@YAXXZ", i32 7}
!43 = !{i32 -2147483584}
!44 = !{i32 -1}
!45 = !DILocation(line: 26, column: 3, scope: !46)
!46 = !DISubprogram(name: "main", scope: !47, file: !47, line: 25, type: !48, isLocal: false, isDefinition: true, scopeLine: 25, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?main@@YAXXZ")
!47 = !DIFile(filename: "hitobject_fromrayquery_scalarrepl.hlsl", directory: "")
!48 = !DISubroutineType(types: !29)
!49 = !DILocation(line: 26, column: 78, scope: !46)
!50 = !DILocation(line: 27, column: 3, scope: !46)
!51 = !DILocation(line: 8, column: 8, scope: !52, inlinedAt: !53)
!52 = !DISubprogram(name: "MakeRayDesc", scope: !47, file: !47, line: 6, type: !48, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false)
!53 = distinct !DILocation(line: 27, column: 17, scope: !46)
!54 = !DILocation(line: 8, column: 15, scope: !52, inlinedAt: !53)
!55 = !{!56, !56, i64 0}
!56 = !{!"omnipotent char", !57, i64 0}
!57 = !{!"Simple C/C++ TBAA"}
!58 = !{!59}
!59 = distinct !{!59, !60, !"\01?MakeRayDesc@@YA?AURayDesc@@XZ: %agg.result"}
!60 = distinct !{!60, !"\01?MakeRayDesc@@YA?AURayDesc@@XZ"}
!61 = !DILocation(line: 9, column: 8, scope: !52, inlinedAt: !53)
!62 = !DILocation(line: 9, column: 18, scope: !52, inlinedAt: !53)
!63 = !DILocation(line: 10, column: 8, scope: !52, inlinedAt: !53)
!64 = !DILocation(line: 10, column: 13, scope: !52, inlinedAt: !53)
!65 = !{!66, !66, i64 0}
!66 = !{!"float", !56, i64 0}
!67 = !DILocation(line: 11, column: 8, scope: !52, inlinedAt: !53)
!68 = !DILocation(line: 11, column: 13, scope: !52, inlinedAt: !53)
!69 = !DILocation(line: 28, column: 3, scope: !46)
!70 = !DILocation(line: 30, column: 7, scope: !46)
!71 = !DILocation(line: 21, column: 3, scope: !72, inlinedAt: !73)
!72 = !DISubprogram(name: "Use", scope: !47, file: !47, line: 20, type: !48, isLocal: false, isDefinition: true, scopeLine: 20, flags: DIFlagPrototyped, isOptimized: false)
!73 = distinct !DILocation(line: 30, column: 3, scope: !46)
!74 = !DILocation(line: 32, column: 3, scope: !46)
!75 = !DILocation(line: 33, column: 13, scope: !46)
!76 = !DILocation(line: 33, column: 9, scope: !46)
!77 = !DILocation(line: 33, column: 11, scope: !46)
!78 = !DILocation(line: 34, column: 13, scope: !46)
!79 = !DILocation(line: 34, column: 9, scope: !46)
!80 = !DILocation(line: 34, column: 11, scope: !46)
!81 = !DILocation(line: 35, column: 7, scope: !46)
!82 = !DILocation(line: 21, column: 3, scope: !72, inlinedAt: !83)
!83 = distinct !DILocation(line: 35, column: 3, scope: !46)
!84 = !DILocation(line: 37, column: 13, scope: !46)
!85 = !DILocation(line: 37, column: 9, scope: !46)
!86 = !DILocation(line: 37, column: 11, scope: !46)
!87 = !DILocation(line: 38, column: 13, scope: !46)
!88 = !DILocation(line: 38, column: 9, scope: !46)
!89 = !DILocation(line: 38, column: 11, scope: !46)
!90 = !DILocation(line: 39, column: 7, scope: !46)
!91 = !DILocation(line: 21, column: 3, scope: !72, inlinedAt: !92)
!92 = distinct !DILocation(line: 39, column: 3, scope: !46)
!93 = !DILocation(line: 41, column: 25, scope: !46)
!94 = !DILocation(line: 41, column: 3, scope: !46)
!95 = !DILocation(line: 42, column: 25, scope: !46)
!96 = !DILocation(line: 42, column: 3, scope: !46)
!97 = !DILocation(line: 43, column: 1, scope: !46)
