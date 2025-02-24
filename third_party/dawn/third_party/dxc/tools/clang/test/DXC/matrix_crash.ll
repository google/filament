; RUN: %dxopt %s -hlsl-passes-resume -hlmatrixlower -S | FileCheck %s


; This test verifies that we convert casting an undef matrix to an undef matrix
; that is then stored. In the absence of this behavior we were encountering an
; asan use-after-free.

; CHECK: store <8 x float> undef, <8 x float>*

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%ConstantBuffer = type opaque
%class.matrix.float.4.2 = type { [4 x <2 x float>] }

@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @main() #0 {
  %_e1 = alloca %class.matrix.float.4.2, align 4
  br label %1, !dbg !17 ; line:2 col:3

; <label>:1                                       ; preds = %1, %0
  br label %1, !dbg !17 ; line:2 col:3

"\01?foo@@YA?AV?$matrix@M$03$01@@XZ.exit":        ; No predecessors!
  %2 = call %class.matrix.float.4.2 @"dx.hl.cast.rowMatToColMat.%class.matrix.float.4.2 (i32, %class.matrix.float.4.2)"(i32 7, %class.matrix.float.4.2 undef), !dbg !23 ; line:9 col:12
  %3 = call %class.matrix.float.4.2 @"dx.hl.matldst.colStore.%class.matrix.float.4.2 (i32, %class.matrix.float.4.2*, %class.matrix.float.4.2)"(i32 1, %class.matrix.float.4.2* %_e1, %class.matrix.float.4.2 %2), !dbg !23 ; line:9 col:12
  ret void, !dbg !24 ; line:10 col:1
}

; Function Attrs: nounwind readnone
declare %class.matrix.float.4.2 @"dx.hl.cast.rowMatToColMat.%class.matrix.float.4.2 (i32, %class.matrix.float.4.2)"(i32, %class.matrix.float.4.2) #1

; Function Attrs: nounwind
declare %class.matrix.float.4.2 @"dx.hl.matldst.colStore.%class.matrix.float.4.2 (i32, %class.matrix.float.4.2*, %class.matrix.float.4.2)"(i32, %class.matrix.float.4.2*, %class.matrix.float.4.2) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!10}
!dx.fnprops = !{!14}
!dx.options = !{!15, !16}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4522 (user/cbieneman/asan-fix, 296e3a37750)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 8}
!5 = !{!"cs", i32 6, i32 0}
!6 = !{i32 1, void ()* @main, !7}
!7 = !{!8}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{void ()* @main, !"main", null, !11, null}
!11 = !{null, null, !12, null}
!12 = !{!13}
!13 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!14 = !{void ()* @main, i32 5, i32 1, i32 1, i32 1}
!15 = !{i32 144}
!16 = !{i32 -1}
!17 = !DILocation(line: 2, column: 3, scope: !18, inlinedAt: !21)
!18 = !DISubprogram(name: "foo", scope: !19, file: !19, line: 1, type: !20, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false)
!19 = !DIFile(filename: "matrix_crash.hlsl", directory: "")
!20 = !DISubroutineType(types: !9)
!21 = distinct !DILocation(line: 9, column: 18, scope: !22)
!22 = !DISubprogram(name: "main", scope: !19, file: !19, line: 8, type: !20, isLocal: false, isDefinition: true, scopeLine: 8, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @main)
!23 = !DILocation(line: 9, column: 12, scope: !22)
!24 = !DILocation(line: 10, column: 1, scope: !22)
