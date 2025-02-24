; RUN: %opt %s -dce -S | FileCheck %s
; Just run a pass that shouldn't do anything in order to pass through assembler

; dxa TempReg.ll -o TempReg.dxl

; CHECK: define void
; CHECK-SAME: sreg
; CHECK-SAME: i32 %n
; CHECK-SAME: i32 %val
; CHECK: call void @dx.op.tempRegStore.i32

; CHECK: define void
; CHECK-SAME: sreg
; CHECK-SAME: i32 %n
; CHECK-SAME: i32 %val
; CHECK: call void @dx.op.tempRegStore.i32

; CHECK: define void
; CHECK-SAME: sreg
; CHECK-SAME: i32 %n
; CHECK-SAME: float %val
; CHECK: call void @dx.op.tempRegStore.f32

; CHECK: define i32
; CHECK-SAME: ureg
; CHECK-SAME: i32 %n
; CHECK: call i32 @dx.op.tempRegLoad.i32

; CHECK: define i32
; CHECK-SAME: ireg
; CHECK-SAME: i32 %n
; CHECK: call i32 @dx.op.tempRegLoad.i32

; CHECK: define float
; CHECK-SAME: freg
; CHECK-SAME: i32 %n
; CHECK: call float @dx.op.tempRegLoad.f32

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: alwaysinline nounwind
define void @"\01?sreg@@YAXII@Z"(i32 %n, i32 %val) #0 {
  call void @dx.op.tempRegStore.i32(i32 1, i32 %n, i32 %val)  ; TempRegStore(index,value)
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?sreg@@YAXIH@Z"(i32 %n, i32 %val) #0 {
  call void @dx.op.tempRegStore.i32(i32 1, i32 %n, i32 %val)  ; TempRegStore(index,value)
  ret void
}

; Function Attrs: alwaysinline nounwind
define void @"\01?sreg@@YAXIM@Z"(i32 %n, float %val) #0 {
  call void @dx.op.tempRegStore.f32(i32 1, i32 %n, float %val)  ; TempRegStore(index,value)
  ret void
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?ureg@@YAII@Z"(i32 %n) #1 {
  %r = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 %n)  ; TempRegLoad(index)
  ret i32 %r
}

; Function Attrs: alwaysinline nounwind readonly
define i32 @"\01?ireg@@YAHI@Z"(i32 %n) #1 {
  %r = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 %n)  ; TempRegLoad(index)
  ret i32 %r
}

; Function Attrs: alwaysinline nounwind readonly
define float @"\01?freg@@YAMI@Z"(i32 %n) #1 {
  %r = call float @dx.op.tempRegLoad.f32(i32 0, i32 %n)  ; TempRegLoad(index)
  ret float %r
}

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.i32(i32, i32, i32) #2

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.f32(i32, i32, float) #2

; Function Attrs: nounwind readonly
declare i32 @dx.op.tempRegLoad.i32(i32, i32) #3

; Function Attrs: nounwind readonly
declare float @dx.op.tempRegLoad.f32(i32, i32) #3

attributes #0 = { alwaysinline nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline nounwind readonly "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.typeAnnotations = !{!5}
!dx.entryPoints = !{!23}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{i32 1, i32 6}
!2 = !{i32 0, i32 0}
!3 = !{!"lib", i32 6, i32 3}
!4 = !{null, null, null, null}
!5 = !{i32 1, void (i32, i32)* @"\01?sreg@@YAXII@Z", !6, void (i32, i32)* @"\01?sreg@@YAXIH@Z", !11, void (i32, float)* @"\01?sreg@@YAXIM@Z", !14, i32 (i32)* @"\01?ureg@@YAII@Z", !17, i32 (i32)* @"\01?ireg@@YAHI@Z", !19, float (i32)* @"\01?freg@@YAMI@Z", !21}
!6 = !{!7, !9, !9}
!7 = !{i32 1, !8, !8}
!8 = !{}
!9 = !{i32 0, !10, !8}
!10 = !{i32 7, i32 5}
!11 = !{!7, !9, !12}
!12 = !{i32 0, !13, !8}
!13 = !{i32 7, i32 4}
!14 = !{!7, !9, !15}
!15 = !{i32 0, !16, !8}
!16 = !{i32 7, i32 9}
!17 = !{!18, !9}
!18 = !{i32 1, !10, !8}
!19 = !{!20, !9}
!20 = !{i32 1, !13, !8}
!21 = !{!22, !9}
!22 = !{i32 1, !16, !8}
!23 = !{null, !"", null, !4, null}
