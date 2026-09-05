; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s

; Based on tools/clang/test/CodeGenDXIL/hlsl/objects/RayQuery/tracerayinline_cb_raydesc.hlsl

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%"$Globals" = type { %struct.RayDesc }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.RayQuery<513, 0>" = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4
@"$Globals" = external constant %"$Globals"

; Function Attrs: nounwind
define void @main() #0 {
entry:

  ; Capture CB, RTAS, and RayQuery
  ; CHECK-DAG: %[[CB:[^ ,]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %"$Globals", %dx.types.ResourceProperties { i32 13, i32 32 })
  ; CHECK-DAG: %[[RTAS:[^ ,]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{[^ ,]+}}, %dx.types.ResourceProperties { i32 16, i32 0 })
  ; CHECK-DAG: %[[RQ:[^ ,]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)

  %0 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32 0, %"$Globals"* @"$Globals", i32 0)
  %1 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32 14, %dx.types.Handle %0, %dx.types.ResourceProperties { i32 13, i32 32 }, %"$Globals" undef)
  %2 = call %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %1, i32 0)
  %3 = getelementptr inbounds %"$Globals", %"$Globals"* %2, i32 0, i32 0
  %rayQuery1 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 513, i32 0), !dbg !34 ; line:12 col:71
  %4 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !38 ; line:13 col:3
  %5 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %4), !dbg !38 ; line:13 col:3
  %6 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !38 ; line:13 col:3

  ; Load RayDesc.Origin
  ; CHECK: %[[ORIG_CB_LD:[^ ,]+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %[[CB]], i32 0)
  ; CHECK: %[[ORIG_EX0:[^ ,]+]] = extractvalue %dx.types.CBufRet.f32 %[[ORIG_CB_LD]], 0
  ; CHECK: %[[ORIG_VX:[^ ,]+]] = insertelement <3 x float> undef, float %[[ORIG_EX0]], i64 0
  ; CHECK: %[[ORIG_EX1:[^ ,]+]] = extractvalue %dx.types.CBufRet.f32 %[[ORIG_CB_LD]], 1
  ; CHECK: %[[ORIG_VXY:[^ ,]+]] = insertelement <3 x float> %[[ORIG_VX]], float %[[ORIG_EX1]], i64 1
  ; CHECK: %[[ORIG_EX2:[^ ,]+]] = extractvalue %dx.types.CBufRet.f32 %[[ORIG_CB_LD]], 2
  ; CHECK: %[[ORIG_VXYZ:[^ ,]+]] = insertelement <3 x float> %[[ORIG_VXY]], float %[[ORIG_EX2]], i64 2

  ; Load RayDesc.TMin
  ; CHECK: %[[TMIN_CB_LD:[^ ,]+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %[[CB]], i32 0)
  ; CHECK: %[[TMIN:[^ ,]+]] = extractvalue %dx.types.CBufRet.f32 %[[TMIN_CB_LD]], 3

  ; Load RayDesc.Direction
  ; CHECK: %[[DIR_CB_LD:[^ ,]+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %[[CB]], i32 1)
  ; CHECK: %[[DIR_EX0:[^ ,]+]] = extractvalue %dx.types.CBufRet.f32 %[[DIR_CB_LD]], 0
  ; CHECK: %[[DIR_VX:[^ ,]+]] = insertelement <3 x float> undef, float %[[DIR_EX0]], i64 0
  ; CHECK: %[[DIR_EX1:[^ ,]+]] = extractvalue %dx.types.CBufRet.f32 %[[DIR_CB_LD]], 1
  ; CHECK: %[[DIR_VXY:[^ ,]+]] = insertelement <3 x float> %[[DIR_VX]], float %[[DIR_EX1]], i64 1
  ; CHECK: %[[DIR_EX2:[^ ,]+]] = extractvalue %dx.types.CBufRet.f32 %[[DIR_CB_LD]], 2
  ; CHECK: %[[DIR_VXYZ:[^ ,]+]] = insertelement <3 x float> %[[DIR_VXY]], float %[[DIR_EX2]], i64 2

  ; Load RayDesc.TMax
  ; CHECK: %21 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %[[CB]], i32 1)
  ; CHECK: %22 = extractvalue %dx.types.CBufRet.f32 %21, 3

  ; Extract RayDesc vector fields
  ; CHECK: %[[ORIGX:[^ ,]+]] = extractelement <3 x float> %[[ORIG_VXYZ]], i64 0
  ; CHECK: %[[ORIGY:[^ ,]+]] = extractelement <3 x float> %[[ORIG_VXYZ]], i64 1
  ; CHECK: %[[ORIGZ:[^ ,]+]] = extractelement <3 x float> %[[ORIG_VXYZ]], i64 2
  ; CHECK: %[[DIRX:[^ ,]+]] = extractelement <3 x float> %[[DIR_VXYZ]], i64 0
  ; CHECK: %[[DIRY:[^ ,]+]] = extractelement <3 x float> %[[DIR_VXYZ]], i64 1
  ; CHECK: %[[DIRZ:[^ ,]+]] = extractelement <3 x float> %[[DIR_VXYZ]], i64 2

  %7 = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %3, i32 0, i32 0, !dbg !38 ; line:13 col:3
  %8 = load <3 x float>, <3 x float>* %7, !dbg !38 ; line:13 col:3
  %9 = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %3, i32 0, i32 1, !dbg !38 ; line:13 col:3
  %10 = load float, float* %9, !dbg !38 ; line:13 col:3
  %11 = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %3, i32 0, i32 2, !dbg !38 ; line:13 col:3
  %12 = load <3 x float>, <3 x float>* %11, !dbg !38 ; line:13 col:3
  %13 = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %3, i32 0, i32 3, !dbg !38 ; line:13 col:3
  %14 = load float, float* %13, !dbg !38 ; line:13 col:3

  ; Call TraceRayInline
  ; CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[RQ]], %dx.types.Handle %[[RTAS]], i32 1, i32 2, float %[[ORIGX]], float %[[ORIGY]], float %[[ORIGZ]], float %[[TMIN]], float %[[DIRX]], float %[[DIRY]], float %[[DIRZ]], float %22)

  call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %rayQuery1, %dx.types.Handle %6, i32 1, i32 2, <3 x float> %8, float %10, <3 x float> %12, float %14), !dbg !38 ; line:13 col:3
  ret void, !dbg !39 ; line:14 col:1
}

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

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float) #0

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
!36 = !DIFile(filename: "tools/clang/test/CodeGenDXIL/hlsl/objects/RayQuery/tracerayinline_cb_raydesc.hlsl", directory: "")
!37 = !DISubroutineType(types: !23)
!38 = !DILocation(line: 13, column: 3, scope: !35)
!39 = !DILocation(line: 14, column: 1, scope: !35)
