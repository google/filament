; RUN: opt < %s -always-inline -S | FileCheck %s

; Test that the debug location is preserved when rewriting an inlined call as an invoke

; CHECK: invoke void @test()
; CHECK-NEXT: to label {{.*}} unwind label {{.*}}, !dbg [[INL_LOC:!.*]]
; CHECK: [[SP:.*]] = !DISubprogram(
; CHECK: [[INL_LOC]] = !DILocation(line: 1, scope: [[SP]], inlinedAt: [[INL_AT:.*]])
; CHECK: [[INL_AT]] = distinct !DILocation(line: 2, scope: [[SP]])

declare void @test()
declare i32 @__gxx_personality_v0(...)

attributes #0 = { alwaysinline }
define void @inl() #0 {
  call void @test(), !dbg !3
  ret void
}

define void @caller() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  invoke void @inl()
    to label %cont unwind label %lpad, !dbg !4

cont:
  ret void

lpad:
  landingpad { i8*, i32 }
    cleanup
  ret void
}

!llvm.module.flags = !{!1}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = !DISubprogram()
!3 = !DILocation(line: 1, scope: !2)
!4 = !DILocation(line: 2, scope: !2)
