; RUN: opt < %s -instsimplify -S | FileCheck %s

define i32 @add1(i32 %x) {
; CHECK-LABEL: @add1(
; (X + -1) + 1 -> X
  %l = add i32 %x, -1
  %r = add i32 %l, 1
  ret i32 %r
; CHECK: ret i32 %x
}

define i32 @and1(i32 %x, i32 %y) {
; CHECK-LABEL: @and1(
; (X & Y) & X -> X & Y
  %l = and i32 %x, %y
  %r = and i32 %l, %x
  ret i32 %r
; CHECK: ret i32 %l
}

define i32 @and2(i32 %x, i32 %y) {
; CHECK-LABEL: @and2(
; X & (X & Y) -> X & Y
  %r = and i32 %x, %y
  %l = and i32 %x, %r
  ret i32 %l
; CHECK: ret i32 %r
}

define i32 @or1(i32 %x, i32 %y) {
; CHECK-LABEL: @or1(
; (X | Y) | X -> X | Y
  %l = or i32 %x, %y
  %r = or i32 %l, %x
  ret i32 %r
; CHECK: ret i32 %l
}

define i32 @or2(i32 %x, i32 %y) {
; CHECK-LABEL: @or2(
; X | (X | Y) -> X | Y
  %r = or i32 %x, %y
  %l = or i32 %x, %r
  ret i32 %l
; CHECK: ret i32 %r
}

define i32 @xor1(i32 %x, i32 %y) {
; CHECK-LABEL: @xor1(
; (X ^ Y) ^ X = Y
  %l = xor i32 %x, %y
  %r = xor i32 %l, %x
  ret i32 %r
; CHECK: ret i32 %y
}

define i32 @xor2(i32 %x, i32 %y) {
; CHECK-LABEL: @xor2(
; X ^ (X ^ Y) = Y
  %r = xor i32 %x, %y
  %l = xor i32 %x, %r
  ret i32 %l
; CHECK: ret i32 %y
}

define i32 @sub1(i32 %x, i32 %y) {
; CHECK-LABEL: @sub1(
  %d = sub i32 %x, %y
  %r = sub i32 %x, %d
  ret i32 %r
; CHECK: ret i32 %y
}

define i32 @sub2(i32 %x) {
; CHECK-LABEL: @sub2(
; X - (X + 1) -> -1
  %xp1 = add i32 %x, 1
  %r = sub i32 %x, %xp1
  ret i32 %r
; CHECK: ret i32 -1
}

define i32 @sub3(i32 %x, i32 %y) {
; CHECK-LABEL: @sub3(
; ((X + 1) + Y) - (Y + 1) -> X
  %xp1 = add i32 %x, 1
  %lhs = add i32 %xp1, %y
  %rhs = add i32 %y, 1
  %r = sub i32 %lhs, %rhs
  ret i32 %r
; CHECK: ret i32 %x
}

define i32 @sdiv1(i32 %x, i32 %y) {
; CHECK-LABEL: @sdiv1(
; (no overflow X * Y) / Y -> X
  %mul = mul nsw i32 %x, %y
  %r = sdiv i32 %mul, %y
  ret i32 %r
; CHECK: ret i32 %x
}

define i32 @sdiv2(i32 %x, i32 %y) {
; CHECK-LABEL: @sdiv2(
; (((X / Y) * Y) / Y) -> X / Y
  %div = sdiv i32 %x, %y
  %mul = mul i32 %div, %y
  %r = sdiv i32 %mul, %y
  ret i32 %r
; CHECK: ret i32 %div
}

define i32 @sdiv3(i32 %x, i32 %y) {
; CHECK-LABEL: @sdiv3(
; (X rem Y) / Y -> 0
  %rem = srem i32 %x, %y
  %div = sdiv i32 %rem, %y
  ret i32 %div
; CHECK: ret i32 0
}

define i32 @sdiv4(i32 %x, i32 %y) {
; CHECK-LABEL: @sdiv4(
; (X / Y) * Y -> X if the division is exact
  %div = sdiv exact i32 %x, %y
  %mul = mul i32 %div, %y
  ret i32 %mul
; CHECK: ret i32 %x
}

define i32 @sdiv5(i32 %x, i32 %y) {
; CHECK-LABEL: @sdiv5(
; Y * (X / Y) -> X if the division is exact
  %div = sdiv exact i32 %x, %y
  %mul = mul i32 %y, %div
  ret i32 %mul
; CHECK: ret i32 %x
}


define i32 @udiv1(i32 %x, i32 %y) {
; CHECK-LABEL: @udiv1(
; (no overflow X * Y) / Y -> X
  %mul = mul nuw i32 %x, %y
  %r = udiv i32 %mul, %y
  ret i32 %r
; CHECK: ret i32 %x
}

define i32 @udiv2(i32 %x, i32 %y) {
; CHECK-LABEL: @udiv2(
; (((X / Y) * Y) / Y) -> X / Y
  %div = udiv i32 %x, %y
  %mul = mul i32 %div, %y
  %r = udiv i32 %mul, %y
  ret i32 %r
; CHECK: ret i32 %div
}

define i32 @udiv3(i32 %x, i32 %y) {
; CHECK-LABEL: @udiv3(
; (X rem Y) / Y -> 0
  %rem = urem i32 %x, %y
  %div = udiv i32 %rem, %y
  ret i32 %div
; CHECK: ret i32 0
}

define i32 @udiv4(i32 %x, i32 %y) {
; CHECK-LABEL: @udiv4(
; (X / Y) * Y -> X if the division is exact
  %div = udiv exact i32 %x, %y
  %mul = mul i32 %div, %y
  ret i32 %mul
; CHECK: ret i32 %x
}

define i32 @udiv5(i32 %x, i32 %y) {
; CHECK-LABEL: @udiv5(
; Y * (X / Y) -> X if the division is exact
  %div = udiv exact i32 %x, %y
  %mul = mul i32 %y, %div
  ret i32 %mul
; CHECK: ret i32 %x
}

define i16 @trunc1(i32 %x) {
; CHECK-LABEL: @trunc1(
  %y = add i32 %x, 1
  %tx = trunc i32 %x to i16
  %ty = trunc i32 %y to i16
  %d = sub i16 %ty, %tx
  ret i16 %d
; CHECK: ret i16 1
}
