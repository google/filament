; RUN: %opt-exe %s -S -dxil-cleanup -verify -o %t.ll.converted
; RUN: fc %t.ll.converted %b.ref

define void @main() {
entry:
  %0 = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  %1 = add i32 %0, 7
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %1)
  %2 = call float @dx.op.tempRegLoad.f32(i32 0, i32 0)
	%val = fadd float %2, 0.5
  call void @dx.op.tempRegStore.f32(i32 1, i32 1, float %val)
  %3 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %3)
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.f32(i32, i32, float) #1
declare void @dx.op.tempRegStore.i32(i32, i32, i32) #1

; Function Attrs: nounwind readonly
declare float @dx.op.tempRegLoad.f32(i32, i32) #2
declare i32 @dx.op.tempRegLoad.i32(i32, i32) #2

; Function Attrs: nounwind
declare void @dx.op.storeOutput.i32(i32, i32, i32, i8, i32) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!dx.version = !{!0}
!dx.shaderModel = !{!1}
!dx.entryPoints = !{!2}
!llvm.ident = !{!10}

!0 = !{i32 1, i32 0}
!1 = !{!"ps", i32 6, i32 0}
!2 = !{void ()* @main, !"main", !3, null, null}
!3 = !{!4, !8, null}
!4 = !{!5, !7}
!5 = !{i32 0, !"A", i8 9, i8 0, !6, i8 2, i32 1, i8 4, i32 0, i8 0, null}
!6 = !{i32 0}
!7 = !{i32 1, !"B", i8 9, i8 0, !6, i8 2, i32 1, i8 4, i32 1, i8 0, null}
!8 = !{!9}
!9 = !{i32 0, !"SV_Target", i8 9, i8 16, !6, i8 0, i32 1, i8 4, i32 0, i8 0, null}
!10 = !{!"dxbc2dxil 1.0"}
