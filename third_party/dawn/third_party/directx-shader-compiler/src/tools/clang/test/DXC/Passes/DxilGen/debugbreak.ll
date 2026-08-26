; REQUIRES: dxil-1-10
; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s

; CHECK: call void @dx.op.debugBreak(i32 -2147483615)

; Generated from:
; dxc -T cs_6_10 -fcgl tools/clang/test/HLSLFileCheckLit/hlsl/intrinsics/basic/debugbreak.hlsl
; Debug info manually stripped.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind
define void @main(<3 x i32> %threadId) #0 {
entry:
  %0 = extractelement <3 x i32> %threadId, i32 0
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:
  call void @"dx.hl.op..void (i32)"(i32 420)
  br label %if.end

if.end:
  ret void
}

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32)"(i32) #0

attributes #0 = { nounwind }

!pauseresume = !{!0}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.typeAnnotations = !{!4}
!dx.entryPoints = !{!10}
!dx.fnprops = !{!14}
!dx.options = !{!15, !16}

!0 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!1 = !{!"dxc(private) 1.9.0.5167 (Debug-Break, c89129305)"}
!2 = !{i32 1, i32 10}
!3 = !{!"cs", i32 6, i32 10}
!4 = !{i32 1, void (<3 x i32>)* @main, !5}
!5 = !{!6, !8}
!6 = !{i32 1, !7, !7}
!7 = !{}
!8 = !{i32 0, !9, !7}
!9 = !{i32 4, !"SV_DispatchThreadID", i32 7, i32 5, i32 13, i32 3}
!10 = !{void (<3 x i32>)* @main, !"main", null, !11, null}
!11 = !{null, null, null, null}
!12 = !{}
!13 = !{i32 0}
!14 = !{void (<3 x i32>)* @main, i32 5, i32 8, i32 8, i32 1}
!15 = !{i32 -2147483584}
!16 = !{i32 -1}
