; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; Based on tools/clang/test/CodeGenDXIL/hlsl/objects/RayQuery/tracerayinline.hlsl,
; with call to DoTrace commented out.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%ConstantBuffer = type opaque
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%"class.RayQuery<513, 0>" = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4
@"$Globals" = external constant %ConstantBuffer

; CHECK: define void @main(float* noalias, <3 x float>, float, <3 x float>, float)

; Function Attrs: nounwind
define float @main(%struct.RayDesc* %rayDesc) #0 {
entry:
  %0 = alloca %struct.RayDesc

  ; Copy flattened RayDesc input to main function
  ; RayDesc fields: %1: Origin, %2: TMin, %3: Direction, %4: TMax
  ; CHECK: store float %4, float* %[[RD3_P0:[^ ,]+]]
  ; CHECK: store <3 x float> %3, <3 x float>* %[[RD2_P0:[^ ,]+]]
  ; CHECK: store float %2, float* %[[RD1_P0:[^ ,]+]]
  ; CHECK: store <3 x float> %1, <3 x float>* %[[RD0_P0:[^ ,]+]]

  ; Copy RayDesc fields again
  ; CHECK: %[[LOAD:[^ ,]+]] = load <3 x float>, <3 x float>* %[[RD0_P0]]
  ; CHECK: store <3 x float> %[[LOAD]], <3 x float>* %[[RD0_P1:[^ ,]+]]
  ; CHECK: %[[LOAD:[^ ,]+]] = load float, float* %[[RD1_P0]]
  ; CHECK: store float %[[LOAD]], float* %[[RD1_P1:[^ ,]+]]
  ; CHECK: %[[LOAD:[^ ,]+]] = load <3 x float>, <3 x float>* %[[RD2_P0]]
  ; CHECK: store <3 x float> %[[LOAD]], <3 x float>* %[[RD2_P1:[^ ,]+]]
  ; CHECK: %[[LOAD:[^ ,]+]] = load float, float* %[[RD3_P0]]
  ; CHECK: store float %[[LOAD]], float* %[[RD3_P1:[^ ,]+]]

  %1 = bitcast %struct.RayDesc* %0 to i8*
  %2 = bitcast %struct.RayDesc* %rayDesc to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %1, i8* %2, i64 32, i32 1, i1 false)

  ; Capture RayQuery ptr and RTAS handle
  ; CHECK: %[[RQ0:[^ ]+]] = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 513, i32 0)
  ; CHECK: store i32 %[[RQ0]], i32* %[[RQ_P0:[^ ,]+]]
  ; CHECK: %[[RTAS:[^ ,]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %{{[^ ,]+}}, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef)

  %rayQuery = alloca %"class.RayQuery<513, 0>", align 4
  %rayQuery1 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 513, i32 0), !dbg !35 ; line:15 col:71
  %3 = getelementptr inbounds %"class.RayQuery<513, 0>", %"class.RayQuery<513, 0>"* %rayQuery, i32 0, i32 0, !dbg !35 ; line:15 col:71
  store i32 %rayQuery1, i32* %3, !dbg !35 ; line:15 col:71
  %4 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !39 ; line:17 col:3
  %5 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %4), !dbg !39 ; line:17 col:3
  %6 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef), !dbg !39 ; line:17 col:3

  ; Copy RayDesc fields again
  ; CHECK: %[[LOAD:[^ ,]+]] = load <3 x float>, <3 x float>* %[[RD0_P1]]
  ; CHECK: store <3 x float> %[[LOAD]], <3 x float>* %[[RD0_P2:[^ ,]+]]
  ; CHECK: %[[LOAD:[^ ,]+]] = load float, float* %[[RD1_P1]]
  ; CHECK: store float %[[LOAD]], float* %[[RD1_P2:[^ ,]+]]
  ; CHECK: %[[LOAD:[^ ,]+]] = load <3 x float>, <3 x float>* %[[RD2_P1]]
  ; CHECK: store <3 x float> %[[LOAD]], <3 x float>* %[[RD2_P2:[^ ,]+]]
  ; CHECK: %[[LOAD:[^ ,]+]] = load float, float* %[[RD3_P1]]
  ; CHECK: store float %[[LOAD]], float* %[[RD3_P2:[^ ,]+]]

  ; Load RayDesc fields for TraceRayInline
  ; CHECK: %[[RD0:[^ ,]+]] = load <3 x float>, <3 x float>* %[[RD0_P2]]
  ; CHECK: %[[RD1:[^ ,]+]] = load float, float* %[[RD1_P2]]
  ; CHECK: %[[RD2:[^ ,]+]] = load <3 x float>, <3 x float>* %[[RD2_P2]]
  ; CHECK: %[[RD3:[^ ,]+]] = load float, float* %[[RD3_P2]]

  ; Load RayQuery
  ; CHECK: %[[RQ:[^ ,]+]] = load i32, i32* %[[RQ_P0]]

  ; TraceRayInline call
  ; CHECK: call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %[[RQ]], %dx.types.Handle %[[RTAS]], i32 1, i32 2, <3 x float> %[[RD0]], float %[[RD1]], <3 x float> %[[RD2]], float %[[RD3]])

  call void @"dx.hl.op..void (i32, %\22class.RayQuery<513, 0>\22*, %dx.types.Handle, i32, i32, %struct.RayDesc*)"(i32 325, %"class.RayQuery<513, 0>"* %rayQuery, %dx.types.Handle %6, i32 1, i32 2, %struct.RayDesc* %0), !dbg !39 ; line:17 col:3
  ret float 0.000000e+00, !dbg !40 ; line:18 col:3
}

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %\22class.RayQuery<513, 0>\22*, %dx.types.Handle, i32, i32, %struct.RayDesc*)"(i32, %"class.RayQuery<513, 0>"*, %dx.types.Handle, i32, i32, %struct.RayDesc*) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind
declare i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32, i32, i32) #0

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !18}
!dx.entryPoints = !{!25}
!dx.fnprops = !{!32}
!dx.options = !{!33, !34}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.14861 (main, 33bc44a3d)"}
!3 = !{i32 1, i32 5}
!4 = !{i32 1, i32 9}
!5 = !{!"vs", i32 6, i32 5}
!6 = !{i32 0, %struct.RayDesc undef, !7, %"class.RayQuery<513, 0>" undef, !12}
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
!18 = !{i32 1, float (%struct.RayDesc*)* @main, !19}
!19 = !{!20, !23}
!20 = !{i32 1, !21, !22}
!21 = !{i32 4, !"OUT", i32 7, i32 9}
!22 = !{}
!23 = !{i32 0, !24, !22}
!24 = !{i32 4, !"RAYDESC"}
!25 = !{float (%struct.RayDesc*)* @main, !"main", null, !26, null}
!26 = !{!27, null, !30, null}
!27 = !{!28}
!28 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !29}
!29 = !{i32 0, i32 4}
!30 = !{!31}
!31 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!32 = !{float (%struct.RayDesc*)* @main, i32 1}
!33 = !{i32 64}
!34 = !{i32 -1}
!35 = !DILocation(line: 15, column: 71, scope: !36)
!36 = !DISubprogram(name: "main", scope: !37, file: !37, line: 14, type: !38, isLocal: false, isDefinition: true, scopeLine: 14, flags: DIFlagPrototyped, isOptimized: false, function: float (%struct.RayDesc*)* @main)
!37 = !DIFile(filename: "tools/clang/test/CodeGenDXIL/hlsl/objects/RayQuery/tracerayinline.hlsl", directory: "")
!38 = !DISubroutineType(types: !22)
!39 = !DILocation(line: 17, column: 3, scope: !36)
!40 = !DILocation(line: 18, column: 3, scope: !36)
