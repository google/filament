; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

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
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; $Globals                          cbuffer      NA          NA     CB0   cb4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%ConstantBuffer = type opaque
%dx.types.HitObject = type { i8* }
%"class.dx::HitObject" = type { i32 }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }

@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
entry:
  %hit = alloca %dx.types.HitObject, align 4
  %tmp = alloca %dx.types.HitObject, align 4
  %ray = alloca %struct.RayDesc, align 4
; CHECK-NOT: alloca %struct.RayDesc
  %tmp2 = alloca %dx.types.HitObject, align 4
; CHECK: %[[HIT0:[^ ]+]] = alloca %dx.types.HitObject, align 4
; CHECK: %[[HIT1:[^ ]+]] = alloca %dx.types.HitObject, align 4
; CHECK: %[[HIT2:[^ ]+]] = alloca %dx.types.HitObject, align 4
  %0 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !23 ; line:42 col:3
  call void @llvm.lifetime.start(i64 4, i8* %0) #0, !dbg !23 ; line:42 col:3
; CHECK:  %[[THIS0:[^ ]+]] = call %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %[[HIT0]])
; CHECK-NOT: %[[THIS0]]
  %1 = call %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %hit), !dbg !27 ; line:42 col:17
  %2 = bitcast %dx.types.HitObject* %tmp to i8*, !dbg !28 ; line:43 col:3
  call void @llvm.lifetime.start(i64 4, i8* %2) #0, !dbg !28 ; line:43 col:3
; CHECK:  call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %[[HIT1]])
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %tmp), !dbg !28 ; line:43 col:3
  %3 = bitcast %dx.types.HitObject* %tmp to i8*, !dbg !28 ; line:43 col:3
  call void @llvm.lifetime.end(i64 4, i8* %3) #0, !dbg !28 ; line:43 col:3
  %4 = bitcast %struct.RayDesc* %ray to i8*, !dbg !29 ; line:44 col:3
  call void @llvm.lifetime.start(i64 32, i8* %4) #0, !dbg !29 ; line:44 col:3
  %5 = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %ray, i32 0, i32 0, !dbg !30 ; line:44 col:17
  store <3 x float> zeroinitializer, <3 x float>* %5, !dbg !30 ; line:44 col:17
  %6 = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %ray, i32 0, i32 1, !dbg !30 ; line:44 col:17
  store float 0.000000e+00, float* %6, !dbg !30 ; line:44 col:17
  %7 = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %ray, i32 0, i32 2, !dbg !30 ; line:44 col:17
  store <3 x float> <float 0.000000e+00, float 1.000000e+00, float 0x3FA99999A0000000>, <3 x float>* %7, !dbg !30 ; line:44 col:17
  %8 = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %ray, i32 0, i32 3, !dbg !30 ; line:44 col:17
  store float 1.000000e+03, float* %8, !dbg !30 ; line:44 col:17
  %9 = bitcast %dx.types.HitObject* %tmp2 to i8*, !dbg !31 ; line:45 col:3
  call void @llvm.lifetime.start(i64 4, i8* %9) #0, !dbg !31 ; line:45 col:3
; CHECK: store <3 x float> zeroinitializer, <3 x float>* %[[pRDO:[^ ]+]],
; CHECK: store float 0.000000e+00, float* %[[pRDTMIN:[^ ]+]],
; CHECK: store <3 x float> <float 0.000000e+00, float 1.000000e+00, float 0x3FA99999A0000000>, <3 x float>* %[[pRDD:[^ ]+]],
; CHECK: store float 1.000000e+03, float* %[[pRDTMAX:[^ ]+]],
; CHECK-DAG: %[[RDO:[^ ]+]] = load <3 x float>, <3 x float>* %[[pRDO]],
; CHECK-DAG: %[[RDTMIN:[^ ]+]] = load float, float* %[[pRDTMIN]],
; CHECK-DAG: %[[RDD:[^ ]+]] = load <3 x float>, <3 x float>* %[[pRDD]],
; CHECK-DAG: %[[RDTMAX:[^ ]+]] = load float, float* %[[pRDTMAX]],
; Copy introduced for RayDesc argument
; CHECK-DAG: store <3 x float> %[[RDO]], <3 x float>* %[[pRDO2:[^ ]+]],
; CHECK-DAG: store float %[[RDTMIN]], float* %[[pRDTMIN2:[^ ]+]],
; CHECK-DAG: store <3 x float> %[[RDD]], <3 x float>* %[[pRDD2:[^ ]+]],
; CHECK-DAG: store float %[[RDTMAX]], float* %[[pRDTMAX2:[^ ]+]],
; CHECK-DAG: %[[RDO2:[^ ]+]] = load <3 x float>, <3 x float>* %[[pRDO2]],
; CHECK-DAG: %[[RDTMIN2:[^ ]+]] = load float, float* %[[pRDTMIN2]],
; CHECK-DAG: %[[RDD2:[^ ]+]] = load <3 x float>, <3 x float>* %[[pRDD2]],
; CHECK-DAG: %[[RDTMAX2:[^ ]+]] = load float, float* %[[pRDTMAX2]],
; CHECK:  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 387, %dx.types.HitObject* %[[HIT2]], i32 0, i32 1, <3 x float> %[[RDO2]], float %[[RDTMIN2]], <3 x float> %[[RDD2]], float %[[RDTMAX2]])
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32, %struct.RayDesc*)"(i32 387, %dx.types.HitObject* %tmp2, i32 0, i32 1, %struct.RayDesc* %ray), !dbg !31 ; line:45 col:3
  %10 = bitcast %dx.types.HitObject* %tmp2 to i8*, !dbg !31 ; line:45 col:3
  call void @llvm.lifetime.end(i64 4, i8* %10) #0, !dbg !31 ; line:45 col:3
  %11 = bitcast %struct.RayDesc* %ray to i8*, !dbg !32 ; line:46 col:1
  call void @llvm.lifetime.end(i64 32, i8* %11) #0, !dbg !32 ; line:46 col:1
  %12 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !32 ; line:46 col:1
  call void @llvm.lifetime.end(i64 4, i8* %12) #0, !dbg !32 ; line:46 col:1
  ret void, !dbg !32 ; line:46 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32, %struct.RayDesc*)"(i32, %dx.types.HitObject*, i32, i32, %struct.RayDesc*) #0

attributes #0 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.typeAnnotations = !{!4, !12}
!dx.entryPoints = !{!16}
!dx.fnprops = !{!20}
!dx.options = !{!21, !22}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{i32 1, i32 9}
!3 = !{!"lib", i32 6, i32 9}
!4 = !{i32 0, %"class.dx::HitObject" undef, !5, %struct.RayDesc undef, !7}
!5 = !{i32 4, !6}
!6 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!7 = !{i32 32, !8, !9, !10, !11}
!8 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9, i32 13, i32 3}
!9 = !{i32 6, !"TMin", i32 3, i32 12, i32 7, i32 9}
!10 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9, i32 13, i32 3}
!11 = !{i32 6, !"TMax", i32 3, i32 28, i32 7, i32 9}
!12 = !{i32 1, void ()* @"\01?main@@YAXXZ", !13}
!13 = !{!14}
!14 = !{i32 1, !15, !15}
!15 = !{}
!16 = !{null, !"", null, !17, null}
!17 = !{null, null, !18, null}
!18 = !{!19}
!19 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!20 = !{void ()* @"\01?main@@YAXXZ", i32 7}
!21 = !{i32 -2147483584}
!22 = !{i32 -1}
!23 = !DILocation(line: 42, column: 3, scope: !24)
!24 = !DISubprogram(name: "main", scope: !25, file: !25, line: 41, type: !26, isLocal: false, isDefinition: true, scopeLine: 41, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?main@@YAXXZ")
!25 = !DIFile(filename: "tools/clang/test/HLSLFileCheck/hlsl/objects/HitObject/hitobject_make_ast.hlsl", directory: "")
!26 = !DISubroutineType(types: !15)
!27 = !DILocation(line: 42, column: 17, scope: !24)
!28 = !DILocation(line: 43, column: 3, scope: !24)
!29 = !DILocation(line: 44, column: 3, scope: !24)
!30 = !DILocation(line: 44, column: 17, scope: !24)
!31 = !DILocation(line: 45, column: 3, scope: !24)
!32 = !DILocation(line: 46, column: 1, scope: !24)