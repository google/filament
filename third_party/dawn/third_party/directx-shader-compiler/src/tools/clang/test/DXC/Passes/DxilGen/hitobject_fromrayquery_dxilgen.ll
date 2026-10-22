; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s
; REQUIRES: dxil-1-9

;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; RTAS                              texture     i32         ras      T0t4294967295,space4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%struct.CustomAttrs = type { float, float }
%dx.types.HitObject = type { i8* }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.RWStructuredBuffer<float>" = type { float }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%"class.dx::HitObject" = type { i32 }
%"class.RayQuery<5, 0>" = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4

; CHECK: %[[ATTRA:[^ ]+]] = alloca %struct.CustomAttrs
; CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[RQH:[^ ]+]], %dx.types.Handle %{{[^ ]+}}, i32 0, i32 255, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 9.999000e+03)
; CHECK: %{{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_FromRayQuery(i32 263, i32 %[[RQH]])
; CHECK: %{{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32 264, i32 %[[RQH]], i32 16, %struct.CustomAttrs* %[[ATTRA]])

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
entry:
  %0 = alloca %struct.CustomAttrs
  %agg.tmp = alloca %dx.types.HitObject, align 4
  %agg.tmp1 = alloca %dx.types.HitObject, align 4
  %q2 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 5, i32 0), !dbg !38 ; line:29 col:78
  %1 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !42 ; line:31 col:3
  %2 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %1), !dbg !42 ; line:31 col:3
  %3 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !42 ; line:31 col:3
  call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %q2, %dx.types.Handle %3, i32 0, i32 255, <3 x float> zeroinitializer, float 0.000000e+00, <3 x float> <float 1.000000e+00, float 0.000000e+00, float 0.000000e+00>, float 9.999000e+03), !dbg !42 ; line:31 col:3
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32)"(i32 363, %dx.types.HitObject* %agg.tmp, i32 %q2), !dbg !43 ; line:33 col:7
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 359, %dx.types.HitObject* %agg.tmp) #0, !dbg !44 ; line:24 col:3
  %.0 = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %0, i32 0, i32 0
  store float 1.000000e+00, float* %.0
  %.1 = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %0, i32 0, i32 1
  store float 2.000000e+00, float* %.1, align 4
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32, %struct.CustomAttrs*)"(i32 363, %dx.types.HitObject* %agg.tmp1, i32 %q2, i32 16, %struct.CustomAttrs* %0), !dbg !47 ; line:36 col:7
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 359, %dx.types.HitObject* %agg.tmp1) #0, !dbg !48 ; line:24 col:3
  ret void, !dbg !49 ; line:37 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind
declare i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32, i32, i32) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32, %struct.CustomAttrs*)"(i32, %dx.types.HitObject*, i32, i32, %struct.CustomAttrs*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32)"(i32, %dx.types.HitObject*, i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.typeAnnotations = !{!4, !26}
!dx.entryPoints = !{!30}
!dx.fnprops = !{!35}
!dx.options = !{!36, !37}

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
!31 = !{!32, null, null, null}
!32 = !{!33}
!33 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !34}
!34 = !{i32 0, i32 4}
!35 = !{void ()* @"\01?main@@YAXXZ", i32 7}
!36 = !{i32 -2147483584}
!37 = !{i32 -1}
!38 = !DILocation(line: 29, column: 78, scope: !39)
!39 = !DISubprogram(name: "main", scope: !40, file: !40, line: 28, type: !41, isLocal: false, isDefinition: true, scopeLine: 28, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?main@@YAXXZ")
!40 = !DIFile(filename: "tools/clang/test/CodeGenDXIL/hlsl/objects/HitObject/hitobject_fromrayquery.hlsl", directory: "")
!41 = !DISubroutineType(types: !29)
!42 = !DILocation(line: 31, column: 3, scope: !39)
!43 = !DILocation(line: 33, column: 7, scope: !39)
!44 = !DILocation(line: 24, column: 3, scope: !45, inlinedAt: !46)
!45 = !DISubprogram(name: "Use", scope: !40, file: !40, line: 23, type: !41, isLocal: false, isDefinition: true, scopeLine: 23, flags: DIFlagPrototyped, isOptimized: false)
!46 = distinct !DILocation(line: 33, column: 3, scope: !39)
!47 = !DILocation(line: 36, column: 7, scope: !39)
!48 = !DILocation(line: 24, column: 3, scope: !45, inlinedAt: !49)
!49 = distinct !DILocation(line: 36, column: 3, scope: !39)
!50 = !DILocation(line: 37, column: 1, scope: !39)
