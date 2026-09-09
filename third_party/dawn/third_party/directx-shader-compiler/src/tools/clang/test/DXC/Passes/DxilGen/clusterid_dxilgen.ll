; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s
; REQUIRES: dxil-1-10

; CHECK-LABEL: define void @"\01?test_cluster_id
; CHECK: call i32 @dx.op.clusterID(i32 -2147483645)

; CHECK-LABEL: define void @"\01?test_rayquery_candidate_cluster_id
; CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 -2147483644, i32 %{{.*}})

; CHECK-LABEL: define void @"\01?test_rayquery_committed_cluster_id
; CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 -2147483643, i32 %{{.*}})

; CHECK-LABEL: define void @"\01?test_hitobject_cluster_id
; CHECK: call i32 @dx.op.hitObject_StateScalar.i32(i32 -2147483642, %dx.types.HitObject

; CHECK-DAG: declare i32 @dx.op.clusterID(i32)
; CHECK-DAG: declare i32 @dx.op.rayQuery_StateScalar.i32(i32, i32)
; CHECK-DAG: declare i32 @dx.op.hitObject_StateScalar.i32(i32, %dx.types.HitObject)

;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; outbuf                                UAV    byte         r/w      U0             u0     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RWByteAddressBuffer = type { i32 }
%struct.Payload = type { float }
%struct.BuiltInTriangleIntersectionAttributes = type { <2 x float> }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RaytracingAccelerationStructure = type { i32 }
%dx.types.HitObject = type { i8* }
%"class.RayQuery<0, 0>" = type { i32 }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%"class.dx::HitObject" = type { i32 }

@"\01?outbuf@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4

; Function Attrs: nounwind
define void @"\01?test_cluster_id@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z"(%struct.Payload* noalias %payload, %struct.BuiltInTriangleIntersectionAttributes* %attr) #0 {
entry:
  %0 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0
  %1 = load float, float* %0
  %2 = call i32 @"dx.hl.op.rn.i32 (i32)"(i32 393), !dbg !38 ; line:56 col:14
  %3 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !dbg !42 ; line:57 col:3
  %4 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %3), !dbg !42 ; line:57 col:3
  %5 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %4, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !42 ; line:57 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32 277, %dx.types.Handle %5, i32 0, i32 %2), !dbg !42 ; line:57 col:3
  %6 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0, !dbg !43 ; line:58 col:1
  store float %1, float* %6, !dbg !43 ; line:58 col:1
  ret void, !dbg !43 ; line:58 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
define void @"\01?test_rayquery_candidate_cluster_id@@YAXXZ"() #0 {
entry:
  %rq3 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 0, i32 0), !dbg !44 ; line:69 col:27
  %0 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !46 ; line:77 col:3
  %1 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %0, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !46 ; line:77 col:3
  call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %rq3, %dx.types.Handle %1, i32 0, i32 255, <3 x float> zeroinitializer, float 0.000000e+00, <3 x float> <float 0.000000e+00, float 0.000000e+00, float 1.000000e+00>, float 1.000000e+03), !dbg !46 ; line:77 col:3
  %2 = call i1 @"dx.hl.op..i1 (i32, i32)"(i32 322, i32 %rq3), !dbg !47 ; line:78 col:3
  %3 = call i32 @"dx.hl.op.ro.i32 (i32, i32)"(i32 394, i32 %rq3), !dbg !48 ; line:79 col:14
  %4 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !dbg !49 ; line:80 col:3
  %5 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %4), !dbg !49 ; line:80 col:3
  %6 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !49 ; line:80 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32 277, %dx.types.Handle %6, i32 4, i32 %3), !dbg !49 ; line:80 col:3
  ret void, !dbg !50 ; line:81 col:1
}

; Function Attrs: nounwind
define void @"\01?test_rayquery_committed_cluster_id@@YAXXZ"() #0 {
entry:
  %rq2 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 0, i32 0), !dbg !51 ; line:92 col:27
  %0 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !53 ; line:100 col:3
  %1 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %0, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !53 ; line:100 col:3
  call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %rq2, %dx.types.Handle %1, i32 0, i32 255, <3 x float> zeroinitializer, float 0.000000e+00, <3 x float> <float 0.000000e+00, float 0.000000e+00, float 1.000000e+00>, float 1.000000e+03), !dbg !53 ; line:100 col:3
  %2 = call i32 @"dx.hl.op.ro.i32 (i32, i32)"(i32 395, i32 %rq2), !dbg !54 ; line:101 col:14
  %3 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !dbg !55 ; line:102 col:3
  %4 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %3), !dbg !55 ; line:102 col:3
  %5 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %4, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !55 ; line:102 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32 277, %dx.types.Handle %5, i32 8, i32 %2), !dbg !55 ; line:102 col:3
  ret void, !dbg !56 ; line:103 col:1
}

; Function Attrs: nounwind
define void @"\01?test_hitobject_cluster_id@@YAXXZ"() #0 {
entry:
  %ho = alloca %dx.types.HitObject, align 4
  %0 = bitcast %dx.types.HitObject* %ho to i8*, !dbg !57 ; line:114 col:3
  call void @llvm.lifetime.start(i64 4, i8* %0) #0, !dbg !57 ; line:114 col:3
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %ho), !dbg !59 ; line:114 col:22
  %1 = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 396, %dx.types.HitObject* %ho), !dbg !60 ; line:115 col:14
  %2 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !dbg !61 ; line:116 col:3
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %2), !dbg !61 ; line:116 col:3
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !61 ; line:116 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32 277, %dx.types.Handle %4, i32 12, i32 %1), !dbg !61 ; line:116 col:3
  %5 = bitcast %dx.types.HitObject* %ho to i8*, !dbg !62 ; line:117 col:1
  call void @llvm.lifetime.end(i64 4, i8* %5) #0, !dbg !62 ; line:117 col:1
  ret void, !dbg !62 ; line:117 col:1
}

; Function Attrs: nounwind readnone
declare i32 @"dx.hl.op.rn.i32 (i32)"(i32) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32, %dx.types.Handle, i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #0

; Function Attrs: nounwind readnone
declare i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #1

; Function Attrs: nounwind
declare i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32, i32, i32) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float) #0

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
!dx.typeAnnotations = !{!4, !21}
!dx.entryPoints = !{!28}
!dx.fnprops = !{!32, !33, !34, !35}
!dx.options = !{!36, !37}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{i32 1, i32 10}
!3 = !{!"lib", i32 6, i32 10}
!4 = !{i32 0, %struct.Payload undef, !5, %struct.BuiltInTriangleIntersectionAttributes undef, !7, %"class.RayQuery<0, 0>" undef, !9, %struct.RayDesc undef, !14, %"class.dx::HitObject" undef, !19}
!5 = !{i32 4, !6}
!6 = !{i32 6, !"dummy", i32 3, i32 0, i32 7, i32 9}
!7 = !{i32 8, !8}
!8 = !{i32 6, !"barycentrics", i32 3, i32 0, i32 7, i32 9, i32 13, i32 2}
!9 = !{i32 4, !10, !11}
!10 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 5}
!11 = !{i32 0, !12}
!12 = !{!13, !13}
!13 = !{i32 1, i64 0}
!14 = !{i32 32, !15, !16, !17, !18}
!15 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9, i32 13, i32 3}
!16 = !{i32 6, !"TMin", i32 3, i32 12, i32 7, i32 9}
!17 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9, i32 13, i32 3}
!18 = !{i32 6, !"TMax", i32 3, i32 28, i32 7, i32 9}
!19 = !{i32 4, !20}
!20 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!21 = !{i32 1, void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?test_cluster_id@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", !22, void ()* @"\01?test_rayquery_candidate_cluster_id@@YAXXZ", !27, void ()* @"\01?test_rayquery_committed_cluster_id@@YAXXZ", !27, void ()* @"\01?test_hitobject_cluster_id@@YAXXZ", !27}
!22 = !{!23, !25, !26}
!23 = !{i32 1, !24, !24}
!24 = !{}
!25 = !{i32 2, !24, !24}
!26 = !{i32 0, !24, !24}
!27 = !{!23}
!28 = !{null, !"", null, !29, null}
!29 = !{null, !30, null, null}
!30 = !{!31}
!31 = !{i32 0, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !"outbuf", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!32 = !{void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?test_cluster_id@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", i32 10, i32 4, i32 8}
!33 = !{void ()* @"\01?test_hitobject_cluster_id@@YAXXZ", i32 7}
!34 = !{void ()* @"\01?test_rayquery_candidate_cluster_id@@YAXXZ", i32 7}
!35 = !{void ()* @"\01?test_rayquery_committed_cluster_id@@YAXXZ", i32 7}
!36 = !{i32 -2147483584}
!37 = !{i32 -1}
!38 = !DILocation(line: 56, column: 14, scope: !39)
!39 = !DISubprogram(name: "test_cluster_id", scope: !40, file: !40, line: 55, type: !41, isLocal: false, isDefinition: true, scopeLine: 55, flags: DIFlagPrototyped, isOptimized: false, function: void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?test_cluster_id@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z")
!40 = !DIFile(filename: "tools/clang/test/CodeGenDXIL/hlsl/intrinsics/clusterid.hlsl", directory: "")
!41 = !DISubroutineType(types: !24)
!42 = !DILocation(line: 57, column: 3, scope: !39)
!43 = !DILocation(line: 58, column: 1, scope: !39)
!44 = !DILocation(line: 69, column: 27, scope: !45)
!45 = !DISubprogram(name: "test_rayquery_candidate_cluster_id", scope: !40, file: !40, line: 68, type: !41, isLocal: false, isDefinition: true, scopeLine: 68, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?test_rayquery_candidate_cluster_id@@YAXXZ")
!46 = !DILocation(line: 77, column: 3, scope: !45)
!47 = !DILocation(line: 78, column: 3, scope: !45)
!48 = !DILocation(line: 79, column: 14, scope: !45)
!49 = !DILocation(line: 80, column: 3, scope: !45)
!50 = !DILocation(line: 81, column: 1, scope: !45)
!51 = !DILocation(line: 92, column: 27, scope: !52)
!52 = !DISubprogram(name: "test_rayquery_committed_cluster_id", scope: !40, file: !40, line: 91, type: !41, isLocal: false, isDefinition: true, scopeLine: 91, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?test_rayquery_committed_cluster_id@@YAXXZ")
!53 = !DILocation(line: 100, column: 3, scope: !52)
!54 = !DILocation(line: 101, column: 14, scope: !52)
!55 = !DILocation(line: 102, column: 3, scope: !52)
!56 = !DILocation(line: 103, column: 1, scope: !52)
!57 = !DILocation(line: 114, column: 3, scope: !58)
!58 = !DISubprogram(name: "test_hitobject_cluster_id", scope: !40, file: !40, line: 113, type: !41, isLocal: false, isDefinition: true, scopeLine: 113, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?test_hitobject_cluster_id@@YAXXZ")
!59 = !DILocation(line: 114, column: 22, scope: !58)
!60 = !DILocation(line: 115, column: 14, scope: !58)
!61 = !DILocation(line: 116, column: 3, scope: !58)
!62 = !DILocation(line: 117, column: 1, scope: !58)
