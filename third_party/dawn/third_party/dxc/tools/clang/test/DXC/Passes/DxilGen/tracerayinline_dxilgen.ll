; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s

; Based on tools/clang/test/CodeGenDXIL/hlsl/objects/RayQuery/tracerayinline.hlsl,
; with call to DoTrace commented out.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%"class.RayQuery<513, 0>" = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #0

; Function Attrs: nounwind
declare i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32, i32, i32) #1

; Function Attrs: nounwind
define void @main(float* noalias, <3 x float>, float, <3 x float>, float) #1 {
entry:

  ; Load RayDesc fields from input
  ; CHECK-DAG: %[[ORIGX_LI:[^ ,]+]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  ; CHECK-DAG: %[[ORIGY_LI:[^ ,]+]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)
  ; CHECK-DAG: %[[ORIGZ_LI:[^ ,]+]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 undef)
  ; CHECK-DAG: %[[TMIN:[^ ,]+]] = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  ; CHECK-DAG: %[[DIRX_LI:[^ ,]+]] = call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 0, i32 undef)
  ; CHECK-DAG: %[[DIRY_LI:[^ ,]+]] = call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 1, i32 undef)
  ; CHECK-DAG: %[[DIRZ_LI:[^ ,]+]] = call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 2, i32 undef)
  ; CHECK-DAG: %[[TMAX:[^ ,]+]] = call float @dx.op.loadInput.f32(i32 4, i32 3, i32 0, i8 0, i32 undef)
  ; CHECK-DAG: %[[ORIG_VX:[^ ,]+]] = insertelement <3 x float> undef, float %[[ORIGX_LI]], i64 0
  ; CHECK-DAG: %[[ORIG_VXY:[^ ,]+]] = insertelement <3 x float> %[[ORIG_VX]], float %[[ORIGY_LI]], i64 1
  ; CHECK-DAG: %[[ORIG_VXYZ:[^ ,]+]] = insertelement <3 x float> %[[ORIG_VXY]], float %[[ORIGZ_LI]], i64 2
  ; CHECK-DAG: %[[DIR_VX:[^ ,]+]] = insertelement <3 x float> undef, float %[[DIRX_LI]], i64 0
  ; CHECK-DAG: %[[DIR_VXY:[^ ,]+]] = insertelement <3 x float> %[[DIR_VX]], float %[[DIRY_LI]], i64 1
  ; CHECK-DAG: %[[DIR_VXYZ:[^ ,]+]] = insertelement <3 x float> %[[DIR_VXY]], float %[[DIRZ_LI]], i64 2

  ; Capture RayQuery and RTAS
  ; CHECK-DAG: %[[RQ:[^ ,]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)
  ; CHECK-DAG: %[[RTAS:[^ ,]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{[^ ,]+}}, %dx.types.ResourceProperties { i32 16, i32 0 })

  %rayQuery1 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 513, i32 0), !dbg !41 ; line:15 col:71
  %5 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !45 ; line:17 col:3
  %6 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %5), !dbg !45 ; line:17 col:3
  %7 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %6, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !45 ; line:17 col:3

  ; Extract RayDesc vector fields
  ; CHECK-DAG: %[[ORIGX:[^ ,]+]] = extractelement <3 x float> %[[ORIG_VXYZ]], i64 0
  ; CHECK-DAG: %[[ORIGY:[^ ,]+]] = extractelement <3 x float> %[[ORIG_VXYZ]], i64 1
  ; CHECK-DAG: %[[ORIGZ:[^ ,]+]] = extractelement <3 x float> %[[ORIG_VXYZ]], i64 2
  ; CHECK-DAG: %[[DIRX:[^ ,]+]] = extractelement <3 x float> %[[DIR_VXYZ]], i64 0
  ; CHECK-DAG: %[[DIRY:[^ ,]+]] = extractelement <3 x float> %[[DIR_VXYZ]], i64 1
  ; CHECK-DAG: %[[DIRZ:[^ ,]+]] = extractelement <3 x float> %[[DIR_VXYZ]], i64 2

  ; Call TraceRayInline
  ; CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[RQ]], %dx.types.Handle %[[RTAS]], i32 1, i32 2, float %[[ORIGX]], float %[[ORIGY]], float %[[ORIGZ]], float %[[TMIN]], float %[[DIRX]], float %[[DIRY]], float %[[DIRZ]], float %[[TMAX]])

  call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %rayQuery1, %dx.types.Handle %7, i32 1, i32 2, <3 x float> %1, float %2, <3 x float> %3, float %4), !dbg !45 ; line:17 col:3
  store float 0.000000e+00, float* %0, !dbg !46 ; line:18 col:3
  ret void, !dbg !46 ; line:18 col:3
}

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !18}
!dx.entryPoints = !{!33}
!dx.fnprops = !{!38}
!dx.options = !{!39, !40}

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
!18 = !{i32 1, void (float*, <3 x float>, float, <3 x float>, float)* @main, !19}
!19 = !{!20, !22, !25, !27, !29, !31}
!20 = !{i32 0, !21, !21}
!21 = !{}
!22 = !{i32 1, !23, !24}
!23 = !{i32 4, !"OUT", i32 7, i32 9}
!24 = !{i32 0}
!25 = !{i32 0, !26, !24}
!26 = !{i32 4, !"RAYDESC", i32 7, i32 9}
!27 = !{i32 0, !26, !28}
!28 = !{i32 1}
!29 = !{i32 0, !26, !30}
!30 = !{i32 2}
!31 = !{i32 0, !26, !32}
!32 = !{i32 3}
!33 = !{void (float*, <3 x float>, float, <3 x float>, float)* @main, !"main", null, !34, null}
!34 = !{!35, null, null, null}
!35 = !{!36}
!36 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !37}
!37 = !{i32 0, i32 4}
!38 = !{void (float*, <3 x float>, float, <3 x float>, float)* @main, i32 1}
!39 = !{i32 64}
!40 = !{i32 -1}
!41 = !DILocation(line: 15, column: 71, scope: !42)
!42 = !DISubprogram(name: "main", scope: !43, file: !43, line: 14, type: !44, isLocal: false, isDefinition: true, scopeLine: 14, flags: DIFlagPrototyped, isOptimized: false, function: void (float*, <3 x float>, float, <3 x float>, float)* @main)
!43 = !DIFile(filename: "tools/clang/test/CodeGenDXIL/hlsl/objects/RayQuery/tracerayinline.hlsl", directory: "")
!44 = !DISubroutineType(types: !21)
!45 = !DILocation(line: 17, column: 3, scope: !42)
!46 = !DILocation(line: 18, column: 3, scope: !42)
