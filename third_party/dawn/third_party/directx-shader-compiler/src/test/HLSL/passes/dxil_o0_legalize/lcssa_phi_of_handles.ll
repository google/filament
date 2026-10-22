; RUN: opt -S -dxil-o0-simplify-inst %s | FileCheck %s

; Make sure we remove a single-value phi node of dxil handles.
; Phis of handles are invalid dxil but we can make it valid by replacing
; the phi with its one incoming value.

; CHECK: %handle = call %dx.types.Handle @dx.op.createHandle
; CHECK-NOT: phi
; CHECK: %load = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %handle, i32 0, i32 0)
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }

define void @main() {
entry:
  %handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 3, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  br label %exit

exit:
  %lcssa = phi %dx.types.Handle [ %handle, %entry ]
  %load = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %lcssa, i32 0, i32 0) 
  ret void
}

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #2

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) #2

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.viewIdState = !{!8}
!dx.entryPoints = !{!9}

!0 = !{!"dxc(private) 1.7.0.14024 (main, 1d6b5627a)"}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 7}
!3 = !{!"vs", i32 6, i32 0}
!4 = !{!5, null, null, null}
!5 = !{!6}
!6 = !{i32 0, i32* undef, !"", i32 0, i32 3, i32 1, i32 12, i32 0, !7}
!7 = !{i32 1, i32 4}
!8 = !{[3 x i32] [i32 1, i32 1, i32 1]}
!9 = !{void ()* @main, !"main", !10, !4, !17}
!10 = !{!11, !15, null}
!11 = !{!12}
!12 = !{i32 0, !"A", i8 5, i8 0, !13, i8 0, i32 1, i8 1, i32 0, i8 0, !14}
!13 = !{i32 0}
!14 = !{i32 3, i32 1}
!15 = !{!16}
!16 = !{i32 0, !"Z", i8 5, i8 0, !13, i8 1, i32 1, i8 1, i32 0, i8 0, !14}
!17 = !{i32 0, i64 17}

