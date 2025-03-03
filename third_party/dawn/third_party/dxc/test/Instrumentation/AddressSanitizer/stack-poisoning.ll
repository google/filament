; RUN: opt < %s -asan -asan-module -asan-use-after-return -S | FileCheck --check-prefix=CHECK-UAR %s
; RUN: opt < %s -asan -asan-module -asan-use-after-return=0 -S | FileCheck --check-prefix=CHECK-PLAIN %s
target datalayout = "e-i64:64-f80:128-s:64-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @Foo(i8*)

define void @Bar() uwtable sanitize_address {
entry:
; CHECK-PLAIN-LABEL: Bar
; CHECK-PLAIN-NOT: label
; CHECK-PLAIN: ret void

; CHECK-UAR-LABEL: Bar
; CHECK-UAR: load i32, i32* @__asan_option_detect_stack_use_after_return
; CHECK-UAR: label
; CHECK-UAR: call i64 @__asan_stack_malloc_1
; CHECK-UAR: label
; CHECK-UAR: call void @Foo
; If LocalStackBase != OrigStackBase
; CHECK-UAR: label
; Then Block: poison the entire frame.
  ; CHECK-UAR: store i64 -723401728380766731
  ; CHECK-UAR: store i64 -723401728380766731
  ; CHECK-UAR: store i8 0
  ; CHECK-UAR-NOT: store
  ; CHECK-UAR: label
; Else Block: no UAR frame. Only unpoison the redzones.
  ; CHECK-UAR: store i64 0
  ; CHECK-UAR: store i32 0
  ; CHECK-UAR-NOT: store
  ; CHECK-UAR: label
; Done, no more stores.
; CHECK-UAR-NOT: store
; CHECK-UAR: ret void

  %x = alloca [20 x i8], align 16
  %arraydecay = getelementptr inbounds [20 x i8], [20 x i8]* %x, i64 0, i64 0
  call void @Foo(i8* %arraydecay)
  ret void
}


