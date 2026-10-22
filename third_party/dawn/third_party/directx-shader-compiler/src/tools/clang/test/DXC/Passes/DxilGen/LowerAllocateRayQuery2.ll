; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s
; generated the IR with:
; ExtractIRForPassTest.py -p dxilgen -o LowerAllocateRayQuery2.ll tools\clang\test\CodeGenDXIL\hlsl\objects\RayQuery\allocateRayQuery2.hlsl -- -T vs_6_9
; Importantly, extraction took place with spirv code-gen enabled

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%"class.RayQuery<1024, 1>" = type { i32 }
%"class.RayQuery<1, 0>" = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind
declare i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32, i32, i32) #0

; Function Attrs: nounwind
define void @main(<3 x float>, float, <3 x float>, float) #0 {
entry:
  ; CHECK: call i32 @dx.op.allocateRayQuery2(i32 258, i32 1024, i32 1)
  %rayQuery12 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 1024, i32 1), !dbg !42 ; line:15 col:79
  %4 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !46 ; line:17 col:3
  %5 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %4), !dbg !46 ; line:17 col:3
  %6 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !46 ; line:17 col:3
  call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %rayQuery12, %dx.types.Handle %6, i32 1024, i32 2, <3 x float> %0, float %1, <3 x float> %2, float %3), !dbg !46 ; line:17 col:3

  ; CHECK: call i32 @dx.op.allocateRayQuery(i32 178, i32 1)
  %rayQuery23 = call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 1, i32 0), !dbg !47 ; line:21 col:35
  %7 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !48 ; line:22 col:3
  %8 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %7), !dbg !48 ; line:22 col:3
  %9 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %8, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !48 ; line:22 col:3
  call void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32 325, i32 %rayQuery23, %dx.types.Handle %9, i32 0, i32 2, <3 x float> %0, float %1, <3 x float> %2, float %3), !dbg !48 ; line:22 col:3
  ret void, !dbg !49 ; line:23 col:1
}

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float)"(i32, i32, %dx.types.Handle, i32, i32, <3 x float>, float, <3 x float>, float) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !21}
!dx.entryPoints = !{!34}
!dx.fnprops = !{!39}
!dx.options = !{!40, !41}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4853 (lowerOMM, ca5df957eb33-dirty)"}
!3 = !{i32 1, i32 9}
!4 = !{!"vs", i32 6, i32 9}
!5 = !{i32 0, %struct.RayDesc undef, !6, %"class.RayQuery<1024, 1>" undef, !11, %"class.RayQuery<1, 0>" undef, !17}
!6 = !{i32 32, !7, !8, !9, !10}
!7 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9, i32 13, i32 3}
!8 = !{i32 6, !"TMin", i32 3, i32 12, i32 7, i32 9}
!9 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9, i32 13, i32 3}
!10 = !{i32 6, !"TMax", i32 3, i32 28, i32 7, i32 9}
!11 = !{i32 4, !12, !13}
!12 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 5}
!13 = !{i32 0, !14}
!14 = !{!15, !16}
!15 = !{i32 1, i64 1024}
!16 = !{i32 1, i64 1}
!17 = !{i32 4, !12, !18}
!18 = !{i32 0, !19}
!19 = !{!16, !20}
!20 = !{i32 1, i64 0}
!21 = !{i32 1, void (<3 x float>, float, <3 x float>, float)* @main, !22}
!22 = !{!23, !25, !28, !30, !32}
!23 = !{i32 0, !24, !24}
!24 = !{}
!25 = !{i32 0, !26, !27}
!26 = !{i32 4, !"RAYDESC", i32 7, i32 9}
!27 = !{i32 0}
!28 = !{i32 0, !26, !29}
!29 = !{i32 1}
!30 = !{i32 0, !26, !31}
!31 = !{i32 2}
!32 = !{i32 0, !26, !33}
!33 = !{i32 3}
!34 = !{void (<3 x float>, float, <3 x float>, float)* @main, !"main", null, !35, null}
!35 = !{!36, null, null, null}
!36 = !{!37}
!37 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !38}
!38 = !{i32 0, i32 4}
!39 = !{void (<3 x float>, float, <3 x float>, float)* @main, i32 1}
!40 = !{i32 -2147483584}
!41 = !{i32 -1}
!42 = !DILocation(line: 15, column: 79, scope: !43)
!43 = !DISubprogram(name: "main", scope: !44, file: !44, line: 11, type: !45, isLocal: false, isDefinition: true, scopeLine: 11, flags: DIFlagPrototyped, isOptimized: false, function: void (<3 x float>, float, <3 x float>, float)* @main)
!44 = !DIFile(filename: "tools\5Cclang\5Ctest\5CCodeGenDXIL\5Chlsl\5Cobjects\5CRayQuery\5CallocateRayQuery2.hlsl", directory: "")
!45 = !DISubroutineType(types: !24)
!46 = !DILocation(line: 17, column: 3, scope: !43)
!47 = !DILocation(line: 21, column: 35, scope: !43)
!48 = !DILocation(line: 22, column: 3, scope: !43)
!49 = !DILocation(line: 23, column: 1, scope: !43)
