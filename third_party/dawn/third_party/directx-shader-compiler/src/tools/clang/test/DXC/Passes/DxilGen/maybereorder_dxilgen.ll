; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s
; REQUIRES: dxil-1-9


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

@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
entry:
  %hit = alloca %dx.types.HitObject, align 4
  %0 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !19 ; line:9 col:3
  call void @llvm.lifetime.start(i64 4, i8* %0) #0, !dbg !19 ; line:9 col:3
; CHECK: %{{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)
  %1 = call %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %hit), !dbg !23 ; line:9 col:17
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*)"(i32 359, %dx.types.HitObject* %hit), !dbg !24 ; line:10 col:3
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32)"(i32 359, %dx.types.HitObject* %hit, i32 241, i32 3), !dbg !25 ; line:11 col:3
  call void @"dx.hl.op..void (i32, i32, i32)"(i32 359, i32 242, i32 7), !dbg !26 ; line:12 col:3
  %2 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !27 ; line:13 col:1
  call void @llvm.lifetime.end(i64 4, i8* %2) #0, !dbg !27 ; line:13 col:1
  ret void, !dbg !27 ; line:13 col:1
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
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32, i32)"(i32, %dx.types.HitObject*, i32, i32) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, i32, i32)"(i32, i32, i32) #0

attributes #0 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !8}
!dx.entryPoints = !{!12}
!dx.fnprops = !{!16}
!dx.options = !{!17, !18}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4840 ser_patch_1 9ffd030b1)"}
!3 = !{i32 1, i32 9}
!4 = !{!"lib", i32 6, i32 9}
!5 = !{i32 0, %"class.dx::HitObject" undef, !6}
!6 = !{i32 4, !7}
!7 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!8 = !{i32 1, void ()* @"\01?main@@YAXXZ", !9}
!9 = !{!10}
!10 = !{i32 1, !11, !11}
!11 = !{}
!12 = !{null, !"", null, !13, null}
!13 = !{null, null, !14, null}
!14 = !{!15}
!15 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!16 = !{void ()* @"\01?main@@YAXXZ", i32 7}
!17 = !{i32 -2147483584}
!18 = !{i32 -1}
!19 = !DILocation(line: 9, column: 3, scope: !20)
!20 = !DISubprogram(name: "main", scope: !21, file: !21, line: 8, type: !22, isLocal: false, isDefinition: true, scopeLine: 8, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?main@@YAXXZ")
!21 = !DIFile(filename: "tools/clang/test/HLSLFileCheck/hlsl/objects/HitObject/maybereorder.hlsl", directory: "")
!22 = !DISubroutineType(types: !11)
!23 = !DILocation(line: 9, column: 17, scope: !20)
!24 = !DILocation(line: 10, column: 3, scope: !20)
!25 = !DILocation(line: 11, column: 3, scope: !20)
!26 = !DILocation(line: 12, column: 3, scope: !20)
!27 = !DILocation(line: 13, column: 1, scope: !20)
