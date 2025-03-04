; RUN: opt -instcombine -S < %s | FileCheck %s

define i32 @test1(i32 %x) nounwind {
  %and = and i32 %x, 31
  %sub = sub i32 63, %and
  ret i32 %sub

; CHECK-LABEL: @test1(
; CHECK-NEXT: and i32 %x, 31
; CHECK-NEXT: xor i32 %and, 63
; CHECK-NEXT: ret
}

declare i32 @llvm.ctlz.i32(i32, i1) nounwind readnone

define i32 @test2(i32 %x) nounwind {
  %count = tail call i32 @llvm.ctlz.i32(i32 %x, i1 true) nounwind readnone
  %sub = sub i32 31, %count
  ret i32 %sub

; CHECK-LABEL: @test2(
; CHECK-NEXT: ctlz
; CHECK-NEXT: xor i32 %count, 31
; CHECK-NEXT: ret
}

define i32 @test3(i32 %x) nounwind {
  %and = and i32 %x, 31
  %sub = xor i32 31, %and
  %add = add i32 %sub, 42
  ret i32 %add

; CHECK-LABEL: @test3(
; CHECK-NEXT: and i32 %x, 31
; CHECK-NEXT: sub nsw i32 73, %and
; CHECK-NEXT: ret
}

define i32 @test4(i32 %x) nounwind {
  %sub = xor i32 %x, 2147483648
  %add = add i32 %sub, 42
  ret i32 %add

; CHECK-LABEL: @test4(
; CHECK-NEXT: add i32 %x, -2147483606
; CHECK-NEXT: ret
}
