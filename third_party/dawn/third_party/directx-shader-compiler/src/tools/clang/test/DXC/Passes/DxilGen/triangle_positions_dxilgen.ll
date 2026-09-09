; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s
; REQUIRES: dxil-1-10

; CHECK-LABEL: define void {{.*}}ClosestHit
; CHECK:   %{{.*}} = call <9 x float> @dx.op.triangleObjectPosition.f32(i32 -2147483641)

; CHECK-LABEL: define void {{.*}}AnyHit
; CHECK:   %{{.*}} = call <9 x float> @dx.op.triangleObjectPosition.f32(i32 -2147483641)

; CHECK-LABEL: define void {{.*}}RayQueryTest
; CHECK: %{{.*}} = call <9 x float> @dx.op.rayQuery_CandidateTriangleObjectPosition.f32(i32 -2147483640, i32 %{{.*}})
; CHECK: %{{.*}} = call <9 x float> @dx.op.rayQuery_CommittedTriangleObjectPosition.f32(i32 -2147483639, i32 %{{.*}})

; CHECK-LABEL: define void {{.*}}HitObjectTest
; CHECK: %{{.*}} = call <9 x float> @dx.op.hitObject_TriangleObjectPosition.f32(i32 -2147483638, %dx.types.HitObject %{{.*}})

; CHECK-DAG: declare <9 x float> @dx.op.triangleObjectPosition.f32(i32)
; CHECK-DAG: declare <9 x float> @dx.op.rayQuery_CommittedTriangleObjectPosition.f32(i32, i32)
; CHECK-DAG: declare <9 x float> @dx.op.rayQuery_CandidateTriangleObjectPosition.f32(i32, i32)
; CHECK-DAG: declare <9 x float> @dx.op.hitObject_TriangleObjectPosition.f32(i32, %dx.types.HitObject)

;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; Scene                             texture     i32         ras      T0             t0     1
; RenderTarget                          UAV     f32          2d      U0             u0     1
; Output                                UAV    byte         r/w      U1             u1     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%"class.RWTexture2D<vector<float, 4> >" = type { <4 x float> }
%struct.RWByteAddressBuffer = type { i32 }
%struct.Payload = type { <4 x float> }
%struct.BuiltInTriangleIntersectionAttributes = type { <2 x float> }
%struct.BuiltInTrianglePositions = type { <3 x float>, <3 x float>, <3 x float> }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.HitObject = type { i8* }
%"class.RayQuery<1, 0>" = type { i32 }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%"class.dx::HitObject" = type { i32 }

@"\01?Scene@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4
@"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A" = external global %"class.RWTexture2D<vector<float, 4> >", align 4
@"\01?Output@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4

; Function Attrs: nounwind
define void @"\01?ClosestHit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z"(%struct.Payload* noalias %payload, %struct.BuiltInTriangleIntersectionAttributes* %attr) #0 {
entry:
  %positions = alloca %struct.BuiltInTrianglePositions, align 4
  %0 = bitcast %struct.BuiltInTrianglePositions* %positions to i8*, !dbg !48 ; line:48 col:5
  call void @llvm.lifetime.start(i64 36, i8* %0) #0, !dbg !48 ; line:48 col:5
  call void @"dx.hl.op..void (i32, %struct.BuiltInTrianglePositions*)"(i32 397, %struct.BuiltInTrianglePositions* %positions), !dbg !52 ; line:48 col:42
  %p0 = getelementptr inbounds %struct.BuiltInTrianglePositions, %struct.BuiltInTrianglePositions* %positions, i32 0, i32 0, !dbg !53 ; line:49 col:38
  %1 = load <3 x float>, <3 x float>* %p0, align 4, !dbg !54 ; line:49 col:28
  %2 = extractelement <3 x float> %1, i32 0, !dbg !54 ; line:49 col:28
  %p1 = getelementptr inbounds %struct.BuiltInTrianglePositions, %struct.BuiltInTrianglePositions* %positions, i32 0, i32 1, !dbg !55 ; line:49 col:54
  %3 = load <3 x float>, <3 x float>* %p1, align 4, !dbg !56 ; line:49 col:44
  %4 = extractelement <3 x float> %3, i32 1, !dbg !56 ; line:49 col:44
  %p2 = getelementptr inbounds %struct.BuiltInTrianglePositions, %struct.BuiltInTrianglePositions* %positions, i32 0, i32 2, !dbg !57 ; line:49 col:70
  %5 = load <3 x float>, <3 x float>* %p2, align 4, !dbg !58 ; line:49 col:60
  %6 = extractelement <3 x float> %5, i32 2, !dbg !58 ; line:49 col:60
  %7 = insertelement <4 x float> undef, float %2, i64 0, !dbg !59 ; line:49 col:27
  %8 = insertelement <4 x float> %7, float %4, i64 1, !dbg !59 ; line:49 col:27
  %9 = insertelement <4 x float> %8, float %6, i64 2, !dbg !59 ; line:49 col:27
  %10 = insertelement <4 x float> %9, float 1.000000e+00, i64 3, !dbg !59 ; line:49 col:27
  %11 = bitcast %struct.BuiltInTrianglePositions* %positions to i8*, !dbg !60 ; line:50 col:1
  call void @llvm.lifetime.end(i64 36, i8* %11) #0, !dbg !60 ; line:50 col:1
  %12 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0, !dbg !60 ; line:50 col:1
  store <4 x float> %10, <4 x float>* %12, !dbg !60 ; line:50 col:1
  ret void, !dbg !60 ; line:50 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
define void @"\01?AnyHit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z"(%struct.Payload* noalias %payload, %struct.BuiltInTriangleIntersectionAttributes* %attr) #0 {
entry:
  %positions = alloca %struct.BuiltInTrianglePositions, align 4
  %0 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0
  %1 = load <4 x float>, <4 x float>* %0
  %2 = bitcast %struct.BuiltInTrianglePositions* %positions to i8*, !dbg !61 ; line:60 col:5
  call void @llvm.lifetime.start(i64 36, i8* %2) #0, !dbg !61 ; line:60 col:5
  call void @"dx.hl.op..void (i32, %struct.BuiltInTrianglePositions*)"(i32 397, %struct.BuiltInTrianglePositions* %positions), !dbg !63 ; line:60 col:42
  %p0 = getelementptr inbounds %struct.BuiltInTrianglePositions, %struct.BuiltInTrianglePositions* %positions, i32 0, i32 0, !dbg !64 ; line:61 col:19
  %3 = load <3 x float>, <3 x float>* %p0, align 4, !dbg !65 ; line:61 col:9
  %4 = extractelement <3 x float> %3, i32 0, !dbg !65 ; line:61 col:9
  %cmp = fcmp ogt float %4, 5.000000e-01, !dbg !66 ; line:61 col:24
  br i1 %cmp, label %if.then, label %if.end, !dbg !65 ; line:61 col:9

if.then:                                          ; preds = %entry
  call void @"dx.hl.op..void (i32)"(i32 26), !dbg !67 ; line:62 col:9
  %5 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0
  store <4 x float> %1, <4 x float>* %5
  ret void

if.end:                                           ; preds = %entry
  %6 = bitcast %struct.BuiltInTrianglePositions* %positions to i8*, !dbg !68 ; line:63 col:1
  call void @llvm.lifetime.end(i64 36, i8* %6) #0, !dbg !68 ; line:63 col:1
  %7 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0, !dbg !68 ; line:63 col:1
  store <4 x float> %1, <4 x float>* %7, !dbg !68 ; line:63 col:1
  ret void, !dbg !68 ; line:63 col:1
}

; Function Attrs: nounwind
define void @RayQueryTest() #0 {
entry:
  %q14 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 1, i32 0), !dbg !69 ; line:76 col:37
  %0 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?Scene@@3URaytracingAccelerationStructure@@A", !dbg !71 ; line:84 col:5
  %1 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %0), !dbg !71 ; line:84 col:5
  %2 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !71 ; line:84 col:5
  call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %q14, %dx.types.Handle %2, i32 0, i32 255, <3 x float> zeroinitializer, float 0.000000e+00, <3 x float> <float 1.000000e+00, float 0.000000e+00, float 0.000000e+00>, float 1.000000e+03), !dbg !71 ; line:84 col:5
  %3 = call i1 @"dx.hl.op..i1 (i32, i32)"(i32 322, i32 %q14), !dbg !72 ; line:86 col:12
  br i1 %3, label %while.body, label %while.end, !dbg !73 ; line:86 col:5

while.body:                                       ; preds = %entry, %while.cond.backedge
  %4 = call i32 @"dx.hl.op.ro.i32 (i32, i32)"(i32 302, i32 %q14), !dbg !74 ; line:87 col:13
  %cmp = icmp eq i32 %4, 0, !dbg !75 ; line:87 col:31
  br i1 %cmp, label %if.then, label %while.cond.backedge, !dbg !74 ; line:87 col:13

if.then:                                          ; preds = %while.body
  %5 = call %struct.BuiltInTrianglePositions* @"dx.hl.op.ro.%struct.BuiltInTrianglePositions* (i32, i32)"(i32 398, i32 %q14), !dbg !76 ; line:88 col:53
  %6 = getelementptr inbounds %struct.BuiltInTrianglePositions, %struct.BuiltInTrianglePositions* %5, i32 0, i32 0, !dbg !76 ; line:88 col:53
  %7 = load <3 x float>, <3 x float>* %6, !dbg !76 ; line:88 col:53
  %8 = extractelement <3 x float> %7, i32 0, !dbg !77 ; line:89 col:36
  %9 = call i32 @"dx.hl.op.rn.i32 (i32, float)"(i32 114, float %8), !dbg !78 ; line:89 col:29
  %10 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?Output@@3URWByteAddressBuffer@@A", !dbg !79 ; line:89 col:13
  %11 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %10), !dbg !79 ; line:89 col:13
  %12 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %11, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !79 ; line:89 col:13
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32 277, %dx.types.Handle %12, i32 0, i32 %9), !dbg !79 ; line:89 col:13
  br label %while.cond.backedge, !dbg !80 ; line:90 col:9

while.cond.backedge:                              ; preds = %if.then, %while.body
  %13 = call i1 @"dx.hl.op..i1 (i32, i32)"(i32 322, i32 %q14), !dbg !72 ; line:86 col:12
  br i1 %13, label %while.body, label %while.end, !dbg !73 ; line:86 col:5

while.end:                                        ; preds = %while.cond.backedge, %entry
  %14 = call i32 @"dx.hl.op.ro.i32 (i32, i32)"(i32 317, i32 %q14), !dbg !81 ; line:93 col:9
  %cmp7 = icmp eq i32 %14, 1, !dbg !82 ; line:93 col:29
  br i1 %cmp7, label %if.then.10, label %if.end.13, !dbg !81 ; line:93 col:9

if.then.10:                                       ; preds = %while.end
  %15 = call %struct.BuiltInTrianglePositions* @"dx.hl.op.ro.%struct.BuiltInTrianglePositions* (i32, i32)"(i32 399, i32 %q14), !dbg !83 ; line:94 col:49
  %16 = getelementptr inbounds %struct.BuiltInTrianglePositions, %struct.BuiltInTrianglePositions* %15, i32 0, i32 1, !dbg !83 ; line:94 col:49
  %17 = load <3 x float>, <3 x float>* %16, !dbg !83 ; line:94 col:49
  %18 = extractelement <3 x float> %17, i32 1, !dbg !84 ; line:95 col:32
  %19 = call i32 @"dx.hl.op.rn.i32 (i32, float)"(i32 114, float %18), !dbg !85 ; line:95 col:25
  %20 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?Output@@3URWByteAddressBuffer@@A", !dbg !86 ; line:95 col:9
  %21 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %20), !dbg !86 ; line:95 col:9
  %22 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %21, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !86 ; line:95 col:9
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32 277, %dx.types.Handle %22, i32 4, i32 %19), !dbg !86 ; line:95 col:9
  br label %if.end.13, !dbg !87 ; line:96 col:5

if.end.13:                                        ; preds = %if.then.10, %while.end
  ret void, !dbg !88 ; line:97 col:1
}

; Function Attrs: nounwind
define void @"\01?HitObjectTest@@YAXXZ"() #0 {
entry:
  %0 = alloca %struct.Payload
  %hit = alloca %dx.types.HitObject, align 4
  %1 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !89 ; line:115 col:5
  call void @llvm.lifetime.start(i64 4, i8* %1) #0, !dbg !89 ; line:115 col:5
  %2 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?Scene@@3URaytracingAccelerationStructure@@A", !dbg !91 ; line:115 col:25
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %2), !dbg !91 ; line:115 col:25
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !91 ; line:115 col:25
  %5 = getelementptr inbounds %struct.Payload, %struct.Payload* %0, i32 0, i32 0, !dbg !91 ; line:115 col:25
  store <4 x float> zeroinitializer, <4 x float>* %5, !dbg !91 ; line:115 col:25
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, <3 x float>, float, <3 x float>, float, %struct.Payload*)"(i32 389, %dx.types.HitObject* %hit, %dx.types.Handle %4, i32 0, i32 -1, i32 0, i32 1, i32 0, <3 x float> zeroinitializer, float 0x3F50624DE0000000, <3 x float> <float 0.000000e+00, float 0.000000e+00, float 1.000000e+00>, float 1.000000e+04, %struct.Payload* %0), !dbg !91 ; line:115 col:25
  %6 = getelementptr inbounds %struct.Payload, %struct.Payload* %0, i32 0, i32 0, !dbg !92 ; line:117 col:9
  %7 = load <4 x float>, <4 x float>* %6, !dbg !92 ; line:117 col:9
  %8 = call i1 @"dx.hl.op.rn.i1 (i32, %dx.types.HitObject*)"(i32 383, %dx.types.HitObject* %hit), !dbg !92 ; line:117 col:9
  br i1 %8, label %if.then, label %if.end, !dbg !92 ; line:117 col:9

if.then:                                          ; preds = %entry
  %9 = call %struct.BuiltInTrianglePositions* @"dx.hl.op.ro.%struct.BuiltInTrianglePositions* (i32, %dx.types.HitObject*)"(i32 400, %dx.types.HitObject* %hit), !dbg !93 ; line:118 col:46
  %10 = getelementptr inbounds %struct.BuiltInTrianglePositions, %struct.BuiltInTrianglePositions* %9, i32 0, i32 0, !dbg !93 ; line:118 col:46
  %11 = load <3 x float>, <3 x float>* %10, !dbg !93 ; line:118 col:46
  %12 = getelementptr inbounds %struct.BuiltInTrianglePositions, %struct.BuiltInTrianglePositions* %9, i32 0, i32 1, !dbg !93 ; line:118 col:46
  %13 = load <3 x float>, <3 x float>* %12, !dbg !93 ; line:118 col:46
  %14 = getelementptr inbounds %struct.BuiltInTrianglePositions, %struct.BuiltInTrianglePositions* %9, i32 0, i32 2, !dbg !93 ; line:118 col:46
  %15 = load <3 x float>, <3 x float>* %14, !dbg !93 ; line:118 col:46
  %add = fadd <3 x float> %11, %13, !dbg !94 ; line:119 col:45
  %add2 = fadd <3 x float> %add, %15, !dbg !95 ; line:119 col:60
  %16 = extractelement <3 x float> %add2, i64 0, !dbg !96 ; line:119 col:31
  %17 = extractelement <3 x float> %add2, i64 1, !dbg !96 ; line:119 col:31
  %18 = extractelement <3 x float> %add2, i64 2, !dbg !96 ; line:119 col:31
  %19 = insertelement <4 x float> undef, float %16, i64 0, !dbg !96 ; line:119 col:31
  %20 = insertelement <4 x float> %19, float %17, i64 1, !dbg !96 ; line:119 col:31
  %21 = insertelement <4 x float> %20, float %18, i64 2, !dbg !96 ; line:119 col:31
  %22 = insertelement <4 x float> %21, float 1.000000e+00, i64 3, !dbg !96 ; line:119 col:31
  br label %if.end, !dbg !97 ; line:120 col:5

if.end:                                           ; preds = %if.then, %entry
  %payload.0.0 = phi <4 x float> [ %22, %if.then ], [ %7, %entry ]
  %23 = call <3 x i32> @"dx.hl.op.rn.<3 x i32> (i32)"(i32 14), !dbg !98 ; line:122 col:18
  %24 = shufflevector <3 x i32> %23, <3 x i32> undef, <2 x i32> <i32 0, i32 1>, !dbg !98 ; line:122 col:18
  %25 = load %"class.RWTexture2D<vector<float, 4> >", %"class.RWTexture2D<vector<float, 4> >"* @"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A", !dbg !99 ; line:122 col:5
  %26 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<float, 4> >\22)"(i32 0, %"class.RWTexture2D<vector<float, 4> >" %25), !dbg !99 ; line:122 col:5
  %27 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<float, 4> >\22)"(i32 14, %dx.types.Handle %26, %dx.types.ResourceProperties { i32 4098, i32 1033 }, %"class.RWTexture2D<vector<float, 4> >" zeroinitializer), !dbg !99 ; line:122 col:5
  %28 = call <4 x float>* @"dx.hl.subscript.[].rn.<4 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %27, <2 x i32> %24), !dbg !99 ; line:122 col:5
  store <4 x float> %payload.0.0, <4 x float>* %28, !dbg !100, !tbaa !101 ; line:122 col:42
  %29 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !104 ; line:123 col:1
  call void @llvm.lifetime.end(i64 4, i8* %29) #0, !dbg !104 ; line:123 col:1
  ret void, !dbg !104 ; line:123 col:1
}

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %struct.BuiltInTrianglePositions*)"(i32, %struct.BuiltInTrianglePositions*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32)"(i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32, %dx.types.Handle, i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind readnone
declare i32 @"dx.hl.op.rn.i32 (i32, float)"(i32, float) #1

; Function Attrs: nounwind readnone
declare i1 @"dx.hl.op.rn.i1 (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #1

; Function Attrs: nounwind readonly
declare %struct.BuiltInTrianglePositions* @"dx.hl.op.ro.%struct.BuiltInTrianglePositions* (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #2

; Function Attrs: nounwind readnone
declare <4 x float>* @"dx.hl.subscript.[].rn.<4 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32, %dx.types.Handle, <2 x i32>) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<float, 4> >\22)"(i32, %"class.RWTexture2D<vector<float, 4> >") #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<float, 4> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWTexture2D<vector<float, 4> >") #1

; Function Attrs: nounwind readnone
declare <3 x i32> @"dx.hl.op.rn.<3 x i32> (i32)"(i32) #1

; Function Attrs: nounwind
declare i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32, i32, i32) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, <3 x float>, float, <3 x float>, float, %struct.Payload*)"(i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, <3 x float>, float, <3 x float>, float, %struct.Payload*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float) #0

; Function Attrs: nounwind readonly
declare %struct.BuiltInTrianglePositions* @"dx.hl.op.ro.%struct.BuiltInTrianglePositions* (i32, i32)"(i32, i32) #2

; Function Attrs: nounwind readonly
declare i32 @"dx.hl.op.ro.i32 (i32, i32)"(i32, i32) #2

; Function Attrs: nounwind
declare i1 @"dx.hl.op..i1 (i32, i32)"(i32, i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.typeAnnotations = !{!4, !26}
!dx.entryPoints = !{!33}
!dx.fnprops = !{!42, !43, !44, !45}
!dx.options = !{!46, !47}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{i32 1, i32 10}
!3 = !{!"lib", i32 6, i32 10}
!4 = !{i32 0, %struct.Payload undef, !5, %struct.BuiltInTriangleIntersectionAttributes undef, !7, %struct.BuiltInTrianglePositions undef, !9, %"class.RayQuery<1, 0>" undef, !13, %struct.RayDesc undef, !19, %"class.dx::HitObject" undef, !24}
!5 = !{i32 16, !6}
!6 = !{i32 6, !"color", i32 3, i32 0, i32 7, i32 9, i32 13, i32 4}
!7 = !{i32 8, !8}
!8 = !{i32 6, !"barycentrics", i32 3, i32 0, i32 7, i32 9, i32 13, i32 2}
!9 = !{i32 44, !10, !11, !12}
!10 = !{i32 6, !"p0", i32 3, i32 0, i32 7, i32 9, i32 13, i32 3}
!11 = !{i32 6, !"p1", i32 3, i32 16, i32 7, i32 9, i32 13, i32 3}
!12 = !{i32 6, !"p2", i32 3, i32 32, i32 7, i32 9, i32 13, i32 3}
!13 = !{i32 4, !14, !15}
!14 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 5}
!15 = !{i32 0, !16}
!16 = !{!17, !18}
!17 = !{i32 1, i64 1}
!18 = !{i32 1, i64 0}
!19 = !{i32 32, !20, !21, !22, !23}
!20 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9, i32 13, i32 3}
!21 = !{i32 6, !"TMin", i32 3, i32 12, i32 7, i32 9}
!22 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9, i32 13, i32 3}
!23 = !{i32 6, !"TMax", i32 3, i32 28, i32 7, i32 9}
!24 = !{i32 4, !25}
!25 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!26 = !{i32 1, void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?ClosestHit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", !27, void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?AnyHit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", !27, void ()* @RayQueryTest, !32, void ()* @"\01?HitObjectTest@@YAXXZ", !32}
!27 = !{!28, !30, !31}
!28 = !{i32 1, !29, !29}
!29 = !{}
!30 = !{i32 2, !29, !29}
!31 = !{i32 0, !29, !29}
!32 = !{!28}
!33 = !{null, !"", null, !34, null}
!34 = !{!35, !38, null, null}
!35 = !{!36}
!36 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?Scene@@3URaytracingAccelerationStructure@@A", !"Scene", i32 0, i32 0, i32 1, i32 16, i32 0, !37}
!37 = !{i32 0, i32 4}
!38 = !{!39, !41}
!39 = !{i32 0, %"class.RWTexture2D<vector<float, 4> >"* @"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A", !"RenderTarget", i32 0, i32 0, i32 1, i32 2, i1 false, i1 false, i1 false, !40}
!40 = !{i32 0, i32 9}
!41 = !{i32 1, %struct.RWByteAddressBuffer* @"\01?Output@@3URWByteAddressBuffer@@A", !"Output", i32 0, i32 1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!42 = !{void ()* @"\01?HitObjectTest@@YAXXZ", i32 7}
!43 = !{void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?ClosestHit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", i32 10, i32 16, i32 8}
!44 = !{void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?AnyHit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", i32 9, i32 16, i32 8}
!45 = !{void ()* @RayQueryTest, i32 5, i32 1, i32 1, i32 1}
!46 = !{i32 -2147483584}
!47 = !{i32 -1}
!48 = !DILocation(line: 48, column: 5, scope: !49)
!49 = !DISubprogram(name: "ClosestHit", scope: !50, file: !50, line: 47, type: !51, isLocal: false, isDefinition: true, scopeLine: 47, flags: DIFlagPrototyped, isOptimized: false, function: void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?ClosestHit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z")
!50 = !DIFile(filename: "../../../tools/clang/test/CodeGenDXIL/hlsl/intrinsics/triangle_object_positions.hlsl", directory: "")
!51 = !DISubroutineType(types: !29)
!52 = !DILocation(line: 48, column: 42, scope: !49)
!53 = !DILocation(line: 49, column: 38, scope: !49)
!54 = !DILocation(line: 49, column: 28, scope: !49)
!55 = !DILocation(line: 49, column: 54, scope: !49)
!56 = !DILocation(line: 49, column: 44, scope: !49)
!57 = !DILocation(line: 49, column: 70, scope: !49)
!58 = !DILocation(line: 49, column: 60, scope: !49)
!59 = !DILocation(line: 49, column: 27, scope: !49)
!60 = !DILocation(line: 50, column: 1, scope: !49)
!61 = !DILocation(line: 60, column: 5, scope: !62)
!62 = !DISubprogram(name: "AnyHit", scope: !50, file: !50, line: 59, type: !51, isLocal: false, isDefinition: true, scopeLine: 59, flags: DIFlagPrototyped, isOptimized: false, function: void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?AnyHit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z")
!63 = !DILocation(line: 60, column: 42, scope: !62)
!64 = !DILocation(line: 61, column: 19, scope: !62)
!65 = !DILocation(line: 61, column: 9, scope: !62)
!66 = !DILocation(line: 61, column: 24, scope: !62)
!67 = !DILocation(line: 62, column: 9, scope: !62)
!68 = !DILocation(line: 63, column: 1, scope: !62)
!69 = !DILocation(line: 76, column: 37, scope: !70)
!70 = !DISubprogram(name: "RayQueryTest", scope: !50, file: !50, line: 75, type: !51, isLocal: false, isDefinition: true, scopeLine: 75, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @RayQueryTest)
!71 = !DILocation(line: 84, column: 5, scope: !70)
!72 = !DILocation(line: 86, column: 12, scope: !70)
!73 = !DILocation(line: 86, column: 5, scope: !70)
!74 = !DILocation(line: 87, column: 13, scope: !70)
!75 = !DILocation(line: 87, column: 31, scope: !70)
!76 = !DILocation(line: 88, column: 53, scope: !70)
!77 = !DILocation(line: 89, column: 36, scope: !70)
!78 = !DILocation(line: 89, column: 29, scope: !70)
!79 = !DILocation(line: 89, column: 13, scope: !70)
!80 = !DILocation(line: 90, column: 9, scope: !70)
!81 = !DILocation(line: 93, column: 9, scope: !70)
!82 = !DILocation(line: 93, column: 29, scope: !70)
!83 = !DILocation(line: 94, column: 49, scope: !70)
!84 = !DILocation(line: 95, column: 32, scope: !70)
!85 = !DILocation(line: 95, column: 25, scope: !70)
!86 = !DILocation(line: 95, column: 9, scope: !70)
!87 = !DILocation(line: 96, column: 5, scope: !70)
!88 = !DILocation(line: 97, column: 1, scope: !70)
!89 = !DILocation(line: 115, column: 5, scope: !90)
!90 = !DISubprogram(name: "HitObjectTest", scope: !50, file: !50, line: 106, type: !51, isLocal: false, isDefinition: true, scopeLine: 106, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?HitObjectTest@@YAXXZ")
!91 = !DILocation(line: 115, column: 25, scope: !90)
!92 = !DILocation(line: 117, column: 9, scope: !90)
!93 = !DILocation(line: 118, column: 46, scope: !90)
!94 = !DILocation(line: 119, column: 45, scope: !90)
!95 = !DILocation(line: 119, column: 60, scope: !90)
!96 = !DILocation(line: 119, column: 31, scope: !90)
!97 = !DILocation(line: 120, column: 5, scope: !90)
!98 = !DILocation(line: 122, column: 18, scope: !90)
!99 = !DILocation(line: 122, column: 5, scope: !90)
!100 = !DILocation(line: 122, column: 42, scope: !90)
!101 = !{!102, !102, i64 0}
!102 = !{!"omnipotent char", !103, i64 0}
!103 = !{!"Simple C/C++ TBAA"}
!104 = !DILocation(line: 123, column: 1, scope: !90)
