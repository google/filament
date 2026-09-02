; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; Based on tools/clang/test/CodeGenDXIL/hlsl/objects/RayQuery/tracerayinline_cb_raydesc.hlsl

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%"$Globals" = type { %struct.RayDesc }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.RayQuery<513, 0>" = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4
@"\01?rayDesc@@3URayDesc@@B" = external constant %struct.RayDesc, align 4
@"$Globals" = external constant %"$Globals"

; Function Attrs: nounwind
define void @main() #0 {
entry:
  %0 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32 0, %"$Globals"* @"$Globals", i32 0)

  ; Capture CB, RayDesc ptr from CB, RTAS, and init RayQuery
  ; CHECK-DAG: %[[CB_H:[^ ,]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32 14, %dx.types.Handle %{{[^ ,]+}}, %dx.types.ResourceProperties { i32 13, i32 32 }, %"$Globals" undef)

  %1 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32 14, %dx.types.Handle %0, %dx.types.ResourceProperties { i32 13, i32 32 }, %"$Globals" undef)

  ; CHECK-DAG: %[[CB_PTR:[^ ,]+]] = call %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %[[CB_H]], i32 0)

  %2 = call %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %1, i32 0)

  ; CHECK-DAG: %[[RAYDESC_PTR:[^ ,]+]] = getelementptr inbounds %"$Globals", %"$Globals"* %[[CB_PTR]], i32 0, i32 0

  %3 = getelementptr inbounds %"$Globals", %"$Globals"* %2, i32 0, i32 0

  ; CHECK-DAG: %[[RQ0:[^ ,]+]] = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 513, i32 0)
  ; CHECK-DAG: store i32 %[[RQ0]], i32* %[[RQ_P0:[^ ,]+]]

  %rayQuery = alloca %"class.RayQuery<513, 0>", align 4
  %rayQuery1 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 513, i32 0), !dbg !34 ; line:12 col:71
  %4 = getelementptr inbounds %"class.RayQuery<513, 0>", %"class.RayQuery<513, 0>"* %rayQuery, i32 0, i32 0, !dbg !34 ; line:12 col:71
  store i32 %rayQuery1, i32* %4, !dbg !34 ; line:12 col:71

  %5 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !38 ; line:13 col:3
  %6 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %5), !dbg !38 ; line:13 col:3

  ; CHECK-DAG: %[[RTAS:[^ ,]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %{{[^ ,]+}}, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef)

  %7 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %6, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef), !dbg !38 ; line:13 col:3

  ; Load RayDesc fields from CB to local copy
  ; CHECK-DAG: %[[ORIG_CBP:[^ ,]+]] = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %[[RAYDESC_PTR]], i32 0, i32 0
  ; CHECK-DAG: %[[ORIG_LD_CB:[^ ,]+]] = load <3 x float>, <3 x float>* %[[ORIG_CBP]]
  ; CHECK-DAG: store <3 x float> %[[ORIG_LD_CB]], <3 x float>* %[[ORIG_P0:[^ ,]+]]
  ; CHECK-DAG: %[[TMIN_CBP:[^ ,]+]] = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %[[RAYDESC_PTR]], i32 0, i32 1
  ; CHECK-DAG: %[[TMIN_LD_CB:[^ ,]+]] = load float, float* %[[TMIN_CBP]]
  ; CHECK-DAG: store float %[[TMIN_LD_CB]], float* %[[TMIN_P0:[^ ,]+]]
  ; CHECK-DAG: %[[DIR_CBP:[^ ,]+]] = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %[[RAYDESC_PTR]], i32 0, i32 2
  ; CHECK-DAG: %[[DIR_LD_CB:[^ ,]+]] = load <3 x float>, <3 x float>* %[[DIR_CBP]]
  ; CHECK-DAG: store <3 x float> %[[DIR_LD_CB]], <3 x float>* %[[DIR_P0:[^ ,]+]]
  ; CHECK-DAG: %[[TMAX_CBP:[^ ,]+]] = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %[[RAYDESC_PTR]], i32 0, i32 3
  ; CHECK-DAG: %[[TMAX_LD_CB:[^ ,]+]] = load float, float* %[[TMAX_CBP]]
  ; CHECK-DAG: store float %[[TMAX_LD_CB]], float* %[[TMAX_P0:[^ ,]+]]

  ; Load RayDesc fields from local copy
  ; CHECK-DAG: %[[ORIG:[^ ,]+]] = load <3 x float>, <3 x float>* %[[ORIG_P0]]
  ; CHECK-DAG: %[[TMIN:[^ ,]+]] = load float, float* %[[TMIN_P0]]
  ; CHECK-DAG: %[[DIR:[^ ,]+]] = load <3 x float>, <3 x float>* %[[DIR_P0]]
  ; CHECK-DAG: %[[TMAX:[^ ,]+]] = load float, float* %[[TMAX_P0]]
  ; CHECK-DAG: %[[RQ:[^ ,]+]] = load i32, i32* %[[RQ_P0]]

  ; Call TraceRayInline
  ; CHECK: call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %[[RQ]], %dx.types.Handle %[[RTAS]], i32 1, i32 2, <3 x float> %[[ORIG]], float %[[TMIN]], <3 x float> %[[DIR]], float %[[TMAX]])

  call void @"dx.hl.op..void (i32, %\22class.RayQuery<513, 0>\22*, %dx.types.Handle, i32, i32, %struct.RayDesc*)"(i32 325, %"class.RayQuery<513, 0>"* %rayQuery, %dx.types.Handle %7, i32 1, i32 2, %struct.RayDesc* %3), !dbg !38 ; line:13 col:3
  ret void, !dbg !39 ; line:14 col:1
}

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %\22class.RayQuery<513, 0>\22*, %dx.types.Handle, i32, i32, %struct.RayDesc*)"(i32, %"class.RayQuery<513, 0>"*, %dx.types.Handle, i32, i32, %struct.RayDesc*) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32, %"$Globals"*, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"$Globals") #1

; Function Attrs: nounwind
declare i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32, i32, i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !20}
!dx.entryPoints = !{!24}
!dx.fnprops = !{!31}
!dx.options = !{!32, !33}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.14861 (main, 33bc44a3d)"}
!3 = !{i32 1, i32 5}
!4 = !{i32 1, i32 9}
!5 = !{!"vs", i32 6, i32 5}
!6 = !{i32 0, %struct.RayDesc undef, !7, %"class.RayQuery<513, 0>" undef, !12, %"$Globals" undef, !18}
!7 = !{i32 32, !8, !9, !10, !11}
!8 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9}
!9 = !{i32 6, !"TMin", i32 3, i32 12, i32 7, i32 9}
!10 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9}
!11 = !{i32 6, !"TMax", i32 3, i32 28, i32 7, i32 9}
!12 = !{i32 4, !13, !14}
!13 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 5}
!14 = !{i32 0, !15}
!15 = !{!16, !17}
!16 = !{i32 1, i64 513}
!17 = !{i32 1, i64 0}
!18 = !{i32 32, !19}
!19 = !{i32 6, !"rayDesc", i32 3, i32 0}
!20 = !{i32 1, void ()* @main, !21}
!21 = !{!22}
!22 = !{i32 1, !23, !23}
!23 = !{}
!24 = !{void ()* @main, !"main", null, !25, null}
!25 = !{!26, null, !29, null}
!26 = !{!27}
!27 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !28}
!28 = !{i32 0, i32 4}
!29 = !{!30}
!30 = !{i32 0, %"$Globals"* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 32, null}
!31 = !{void ()* @main, i32 1}
!32 = !{i32 64}
!33 = !{i32 -1}
!34 = !DILocation(line: 12, column: 71, scope: !35)
!35 = !DISubprogram(name: "main", scope: !36, file: !36, line: 11, type: !37, isLocal: false, isDefinition: true, scopeLine: 11, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @main)
!36 = !DIFile(filename: "/home/texr/git/dxc/main/tools/clang/test/CodeGenDXIL/hlsl/objects/RayQuery/tracerayinline_cb_raydesc.hlsl", directory: "")
!37 = !DISubroutineType(types: !23)
!38 = !DILocation(line: 13, column: 3, scope: !35)
!39 = !DILocation(line: 14, column: 1, scope: !35)
