; RUN: opt -inline -S < %s | FileCheck %s

declare void @use(i8* %a)

define void @helper() {
  %a = alloca i8
  call void @use(i8* %a)
  ret void
}

; Size in llvm.lifetime.X should be 1 (default for i8).
define void @test() {
; CHECK-LABEL: @test(
; CHECK-NOT: lifetime
; CHECK: llvm.lifetime.start(i64 1
; CHECK-NOT: lifetime
; CHECK: llvm.lifetime.end(i64 1
  call void @helper()
; CHECK-NOT: lifetime
; CHECK: ret void
  ret void
}

