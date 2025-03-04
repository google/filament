; RUN: opt -instcombine -S < %s | FileCheck %s

%overflow.result = type {i8, i1}
%ov.result.32 = type { i32, i1 }


declare %overflow.result @llvm.uadd.with.overflow.i8(i8, i8) nounwind readnone
declare %overflow.result @llvm.umul.with.overflow.i8(i8, i8) nounwind readnone
declare %ov.result.32 @llvm.sadd.with.overflow.i32(i32, i32) nounwind readnone
declare %ov.result.32 @llvm.uadd.with.overflow.i32(i32, i32) nounwind readnone
declare %ov.result.32 @llvm.ssub.with.overflow.i32(i32, i32) nounwind readnone
declare %ov.result.32 @llvm.usub.with.overflow.i32(i32, i32) nounwind readnone
declare %ov.result.32 @llvm.smul.with.overflow.i32(i32, i32) nounwind readnone
declare %ov.result.32 @llvm.umul.with.overflow.i32(i32, i32) nounwind readnone
declare double @llvm.powi.f64(double, i32) nounwind readonly
declare i32 @llvm.cttz.i32(i32, i1) nounwind readnone
declare i32 @llvm.ctlz.i32(i32, i1) nounwind readnone
declare i32 @llvm.ctpop.i32(i32) nounwind readnone
declare i8 @llvm.ctlz.i8(i8, i1) nounwind readnone
declare double @llvm.cos.f64(double %Val) nounwind readonly
declare double @llvm.sin.f64(double %Val) nounwind readonly

define i8 @uaddtest1(i8 %A, i8 %B) {
  %x = call %overflow.result @llvm.uadd.with.overflow.i8(i8 %A, i8 %B)
  %y = extractvalue %overflow.result %x, 0
  ret i8 %y
; CHECK-LABEL: @uaddtest1(
; CHECK-NEXT: %y = add i8 %A, %B
; CHECK-NEXT: ret i8 %y
}

define i8 @uaddtest2(i8 %A, i8 %B, i1* %overflowPtr) {
  %and.A = and i8 %A, 127
  %and.B = and i8 %B, 127
  %x = call %overflow.result @llvm.uadd.with.overflow.i8(i8 %and.A, i8 %and.B)
  %y = extractvalue %overflow.result %x, 0
  %z = extractvalue %overflow.result %x, 1
  store i1 %z, i1* %overflowPtr
  ret i8 %y
; CHECK-LABEL: @uaddtest2(
; CHECK-NEXT: %and.A = and i8 %A, 127
; CHECK-NEXT: %and.B = and i8 %B, 127
; CHECK-NEXT: %x = add nuw i8 %and.A, %and.B
; CHECK-NEXT: store i1 false, i1* %overflowPtr
; CHECK-NEXT: ret i8 %x
}

define i8 @uaddtest3(i8 %A, i8 %B, i1* %overflowPtr) {
  %or.A = or i8 %A, -128
  %or.B = or i8 %B, -128
  %x = call %overflow.result @llvm.uadd.with.overflow.i8(i8 %or.A, i8 %or.B)
  %y = extractvalue %overflow.result %x, 0
  %z = extractvalue %overflow.result %x, 1
  store i1 %z, i1* %overflowPtr
  ret i8 %y
; CHECK-LABEL: @uaddtest3(
; CHECK-NEXT: %or.A = or i8 %A, -128
; CHECK-NEXT: %or.B = or i8 %B, -128
; CHECK-NEXT: %x = add i8 %or.A, %or.B
; CHECK-NEXT: store i1 true, i1* %overflowPtr
; CHECK-NEXT: ret i8 %x
}

define i8 @uaddtest4(i8 %A, i1* %overflowPtr) {
  %x = call %overflow.result @llvm.uadd.with.overflow.i8(i8 undef, i8 %A)
  %y = extractvalue %overflow.result %x, 0
  %z = extractvalue %overflow.result %x, 1
  store i1 %z, i1* %overflowPtr
  ret i8 %y
; CHECK-LABEL: @uaddtest4(
; CHECK-NEXT: ret i8 undef
}

define i8 @uaddtest5(i8 %A, i1* %overflowPtr) {
  %x = call %overflow.result @llvm.uadd.with.overflow.i8(i8 0, i8 %A)
  %y = extractvalue %overflow.result %x, 0
  %z = extractvalue %overflow.result %x, 1
  store i1 %z, i1* %overflowPtr
  ret i8 %y
; CHECK-LABEL: @uaddtest5(
; CHECK: ret i8 %A
}

define i1 @uaddtest6(i8 %A, i8 %B) {
  %x = call %overflow.result @llvm.uadd.with.overflow.i8(i8 %A, i8 -4)
  %z = extractvalue %overflow.result %x, 1
  ret i1 %z
; CHECK-LABEL: @uaddtest6(
; CHECK-NEXT: %z = icmp ugt i8 %A, 3
; CHECK-NEXT: ret i1 %z
}

define i8 @uaddtest7(i8 %A, i8 %B) {
  %x = call %overflow.result @llvm.uadd.with.overflow.i8(i8 %A, i8 %B)
  %z = extractvalue %overflow.result %x, 0
  ret i8 %z
; CHECK-LABEL: @uaddtest7(
; CHECK-NEXT: %z = add i8 %A, %B
; CHECK-NEXT: ret i8 %z
}

; PR20194
define %ov.result.32 @saddtest_nsw(i8 %a, i8 %b) {
  %A = sext i8 %a to i32
  %B = sext i8 %b to i32
  %x = call %ov.result.32 @llvm.sadd.with.overflow.i32(i32 %A, i32 %B)
  ret %ov.result.32 %x
; CHECK-LABEL: @saddtest_nsw
; CHECK: %x = add nsw i32 %A, %B
; CHECK-NEXT: %1 = insertvalue %ov.result.32 { i32 undef, i1 false }, i32 %x, 0
; CHECK-NEXT:  ret %ov.result.32 %1
}

define %ov.result.32 @uaddtest_nuw(i32 %a, i32 %b) {
  %A = and i32 %a, 2147483647
  %B = and i32 %b, 2147483647
  %x = call %ov.result.32 @llvm.uadd.with.overflow.i32(i32 %A, i32 %B)
  ret %ov.result.32 %x
; CHECK-LABEL: @uaddtest_nuw
; CHECK: %x = add nuw i32 %A, %B
; CHECK-NEXT: %1 = insertvalue %ov.result.32 { i32 undef, i1 false }, i32 %x, 0
; CHECK-NEXT:  ret %ov.result.32 %1
}

define %ov.result.32 @ssubtest_nsw(i8 %a, i8 %b) {
  %A = sext i8 %a to i32
  %B = sext i8 %b to i32
  %x = call %ov.result.32 @llvm.ssub.with.overflow.i32(i32 %A, i32 %B)
  ret %ov.result.32 %x
; CHECK-LABEL: @ssubtest_nsw
; CHECK: %x = sub nsw i32 %A, %B
; CHECK-NEXT: %1 = insertvalue %ov.result.32 { i32 undef, i1 false }, i32 %x, 0
; CHECK-NEXT:  ret %ov.result.32 %1
}

define %ov.result.32 @usubtest_nuw(i32 %a, i32 %b) {
  %A = or i32 %a, 2147483648
  %B = and i32 %b, 2147483647
  %x = call %ov.result.32 @llvm.usub.with.overflow.i32(i32 %A, i32 %B)
  ret %ov.result.32 %x
; CHECK-LABEL: @usubtest_nuw
; CHECK: %x = sub nuw i32 %A, %B
; CHECK-NEXT: %1 = insertvalue %ov.result.32 { i32 undef, i1 false }, i32 %x, 0
; CHECK-NEXT:  ret %ov.result.32 %1
}

define %ov.result.32 @smultest1_nsw(i32 %a, i32 %b) {
  %A = and i32 %a, 4095 ; 0xfff
  %B = and i32 %b, 524287; 0x7ffff
  %x = call %ov.result.32 @llvm.smul.with.overflow.i32(i32 %A, i32 %B)
  ret %ov.result.32 %x
; CHECK-LABEL: @smultest1_nsw
; CHECK: %x = mul nuw nsw i32 %A, %B
; CHECK-NEXT: %1 = insertvalue %ov.result.32 { i32 undef, i1 false }, i32 %x, 0
; CHECK-NEXT:  ret %ov.result.32 %1
}

define %ov.result.32 @smultest2_nsw(i32 %a, i32 %b) {
  %A = ashr i32 %a, 16
  %B = ashr i32 %b, 16
  %x = call %ov.result.32 @llvm.smul.with.overflow.i32(i32 %A, i32 %B)
  ret %ov.result.32 %x
; CHECK-LABEL: @smultest2_nsw
; CHECK: %x = mul nsw i32 %A, %B
; CHECK-NEXT: %1 = insertvalue %ov.result.32 { i32 undef, i1 false }, i32 %x, 0
; CHECK-NEXT:  ret %ov.result.32 %1
}

define %ov.result.32 @smultest3_sw(i32 %a, i32 %b) {
  %A = ashr i32 %a, 16
  %B = ashr i32 %b, 15
  %x = call %ov.result.32 @llvm.smul.with.overflow.i32(i32 %A, i32 %B)
  ret %ov.result.32 %x
; CHECK-LABEL: @smultest3_sw
; CHECK: %x = call %ov.result.32 @llvm.smul.with.overflow.i32(i32 %A, i32 %B)
; CHECK-NEXT:  ret %ov.result.32 %x
}

define %ov.result.32 @umultest_nuw(i32 %a, i32 %b) {
  %A = and i32 %a, 65535 ; 0xffff
  %B = and i32 %b, 65535 ; 0xffff
  %x = call %ov.result.32 @llvm.umul.with.overflow.i32(i32 %A, i32 %B)
  ret %ov.result.32 %x
; CHECK-LABEL: @umultest_nuw
; CHECK: %x = mul nuw i32 %A, %B
; CHECK-NEXT: %1 = insertvalue %ov.result.32 { i32 undef, i1 false }, i32 %x, 0
; CHECK-NEXT:  ret %ov.result.32 %1
}

define i8 @umultest1(i8 %A, i1* %overflowPtr) {
  %x = call %overflow.result @llvm.umul.with.overflow.i8(i8 0, i8 %A)
  %y = extractvalue %overflow.result %x, 0
  %z = extractvalue %overflow.result %x, 1
  store i1 %z, i1* %overflowPtr
  ret i8 %y
; CHECK-LABEL: @umultest1(
; CHECK-NEXT: store i1 false, i1* %overflowPtr
; CHECK-NEXT: ret i8 0
}

define i8 @umultest2(i8 %A, i1* %overflowPtr) {
  %x = call %overflow.result @llvm.umul.with.overflow.i8(i8 1, i8 %A)
  %y = extractvalue %overflow.result %x, 0
  %z = extractvalue %overflow.result %x, 1
  store i1 %z, i1* %overflowPtr
  ret i8 %y
; CHECK-LABEL: @umultest2(
; CHECK-NEXT: store i1 false, i1* %overflowPtr
; CHECK-NEXT: ret i8 %A
}

define i32 @umultest3(i32 %n) nounwind {
  %shr = lshr i32 %n, 2
  %mul = call %ov.result.32 @llvm.umul.with.overflow.i32(i32 %shr, i32 3)
  %ov = extractvalue %ov.result.32 %mul, 1
  %res = extractvalue %ov.result.32 %mul, 0
  %ret = select i1 %ov, i32 -1, i32 %res
  ret i32 %ret
; CHECK-LABEL: @umultest3(
; CHECK-NEXT: shr
; CHECK-NEXT: mul nuw
; CHECK-NEXT: ret
}

define i32 @umultest4(i32 %n) nounwind {
  %shr = lshr i32 %n, 1
  %mul = call %ov.result.32 @llvm.umul.with.overflow.i32(i32 %shr, i32 4)
  %ov = extractvalue %ov.result.32 %mul, 1
  %res = extractvalue %ov.result.32 %mul, 0
  %ret = select i1 %ov, i32 -1, i32 %res
  ret i32 %ret
; CHECK-LABEL: @umultest4(
; CHECK: umul.with.overflow
}

define %ov.result.32 @umultest5(i32 %x, i32 %y) nounwind {
  %or_x = or i32 %x, 2147483648
  %or_y = or i32 %y, 2147483648
  %mul = call %ov.result.32 @llvm.umul.with.overflow.i32(i32 %or_x, i32 %or_y)
  ret %ov.result.32 %mul
; CHECK-LABEL: @umultest5(
; CHECK-NEXT: %[[or_x:.*]] = or i32 %x, -2147483648
; CHECK-NEXT: %[[or_y:.*]] = or i32 %y, -2147483648
; CHECK-NEXT: %[[mul:.*]] = mul i32 %[[or_x]], %[[or_y]]
; CHECK-NEXT: %[[ret:.*]] = insertvalue %ov.result.32 { i32 undef, i1 true }, i32 %[[mul]], 0
; CHECK-NEXT: ret %ov.result.32 %[[ret]]
}

define void @powi(double %V, double *%P) {
entry:
  %A = tail call double @llvm.powi.f64(double %V, i32 -1) nounwind
  store volatile double %A, double* %P

  %B = tail call double @llvm.powi.f64(double %V, i32 0) nounwind
  store volatile double %B, double* %P

  %C = tail call double @llvm.powi.f64(double %V, i32 1) nounwind
  store volatile double %C, double* %P
  ret void
; CHECK-LABEL: @powi(
; CHECK: %A = fdiv double 1.0{{.*}}, %V
; CHECK: store volatile double %A, 
; CHECK: store volatile double 1.0 
; CHECK: store volatile double %V
}

define i32 @cttz(i32 %a) {
entry:
  %or = or i32 %a, 8
  %and = and i32 %or, -8
  %count = tail call i32 @llvm.cttz.i32(i32 %and, i1 true) nounwind readnone
  ret i32 %count
; CHECK-LABEL: @cttz(
; CHECK-NEXT: entry:
; CHECK-NEXT: ret i32 3
}

define i8 @ctlz(i8 %a) {
entry:
  %or = or i8 %a, 32
  %and = and i8 %or, 63
  %count = tail call i8 @llvm.ctlz.i8(i8 %and, i1 true) nounwind readnone
  ret i8 %count
; CHECK-LABEL: @ctlz(
; CHECK-NEXT: entry:
; CHECK-NEXT: ret i8 2
}

define void @cmp.simplify(i32 %a, i32 %b, i1* %c) {
entry:
  %lz = tail call i32 @llvm.ctlz.i32(i32 %a, i1 false) nounwind readnone
  %lz.cmp = icmp eq i32 %lz, 32
  store volatile i1 %lz.cmp, i1* %c
  %tz = tail call i32 @llvm.cttz.i32(i32 %a, i1 false) nounwind readnone
  %tz.cmp = icmp ne i32 %tz, 32
  store volatile i1 %tz.cmp, i1* %c
  %pop = tail call i32 @llvm.ctpop.i32(i32 %b) nounwind readnone
  %pop.cmp = icmp eq i32 %pop, 0
  store volatile i1 %pop.cmp, i1* %c
  ret void
; CHECK: @cmp.simplify
; CHECK-NEXT: entry:
; CHECK-NEXT: %lz.cmp = icmp eq i32 %a, 0
; CHECK-NEXT: store volatile i1 %lz.cmp, i1* %c
; CHECK-NEXT: %tz.cmp = icmp ne i32 %a, 0
; CHECK-NEXT: store volatile i1 %tz.cmp, i1* %c
; CHECK-NEXT: %pop.cmp = icmp eq i32 %b, 0
; CHECK-NEXT: store volatile i1 %pop.cmp, i1* %c
}

define i32 @cttz_simplify1a(i32 %x) nounwind readnone ssp {
  %tmp1 = tail call i32 @llvm.ctlz.i32(i32 %x, i1 false)
  %shr3 = lshr i32 %tmp1, 5
  ret i32 %shr3

; CHECK-LABEL: @cttz_simplify1a(
; CHECK: icmp eq i32 %x, 0
; CHECK-NEXT: zext i1
; CHECK-NEXT: ret i32
}

define i32 @cttz_simplify1b(i32 %x) nounwind readnone ssp {
  %tmp1 = tail call i32 @llvm.ctlz.i32(i32 %x, i1 true)
  %shr3 = lshr i32 %tmp1, 5
  ret i32 %shr3

; CHECK-LABEL: @cttz_simplify1b(
; CHECK-NEXT: ret i32 0
}

define i32 @ctlz_undef(i32 %Value) nounwind {
  %ctlz = call i32 @llvm.ctlz.i32(i32 0, i1 true)
  ret i32 %ctlz

; CHECK-LABEL: @ctlz_undef(
; CHECK-NEXT: ret i32 undef
}

define i32 @cttz_undef(i32 %Value) nounwind {
  %cttz = call i32 @llvm.cttz.i32(i32 0, i1 true)
  ret i32 %cttz

; CHECK-LABEL: @cttz_undef(
; CHECK-NEXT: ret i32 undef
}

define i32 @ctlz_select(i32 %Value) nounwind {
  %tobool = icmp ne i32 %Value, 0
  %ctlz = call i32 @llvm.ctlz.i32(i32 %Value, i1 true)
  %s = select i1 %tobool, i32 %ctlz, i32 32
  ret i32 %s

; CHECK-LABEL: @ctlz_select(
; CHECK-NEXT: call i32 @llvm.ctlz.i32(i32 %Value, i1 false)
; CHECK-NEXT: ret i32
}

define i32 @cttz_select(i32 %Value) nounwind {
  %tobool = icmp ne i32 %Value, 0
  %cttz = call i32 @llvm.cttz.i32(i32 %Value, i1 true)
  %s = select i1 %tobool, i32 %cttz, i32 32
  ret i32 %s

; CHECK-LABEL: @cttz_select(
; CHECK-NEXT: call i32 @llvm.cttz.i32(i32 %Value, i1 false)
; CHECK-NEXT: ret i32
}

; CHECK-LABEL: @overflow_div_add(
; CHECK: ret i1 false
define i1 @overflow_div_add(i32 %v1, i32 %v2) nounwind {
entry:
  %div = sdiv i32 %v1, 2
  %t = call %ov.result.32 @llvm.sadd.with.overflow.i32(i32 %div, i32 1)
  %obit = extractvalue %ov.result.32 %t, 1
  ret i1 %obit
}

; CHECK-LABEL: @overflow_div_sub(
; CHECK: ret i1 false
define i1 @overflow_div_sub(i32 %v1, i32 %v2) nounwind {
entry:
  ; Check cases where the known sign bits are larger than the word size.
  %a = ashr i32 %v1, 18
  %div = sdiv i32 %a, 65536
  %t = call %ov.result.32 @llvm.ssub.with.overflow.i32(i32 %div, i32 1)
  %obit = extractvalue %ov.result.32 %t, 1
  ret i1 %obit
}

; CHECK-LABEL: @overflow_mod_mul(
; CHECK: ret i1 false
define i1 @overflow_mod_mul(i32 %v1, i32 %v2) nounwind {
entry:
  %rem = srem i32 %v1, 1000
  %t = call %ov.result.32 @llvm.smul.with.overflow.i32(i32 %rem, i32 %rem)
  %obit = extractvalue %ov.result.32 %t, 1
  ret i1 %obit
}

; CHECK-LABEL: @overflow_mod_overflow_mul(
; CHECK-NOT: ret i1 false
define i1 @overflow_mod_overflow_mul(i32 %v1, i32 %v2) nounwind {
entry:
  %rem = srem i32 %v1, 65537
  ; This may overflow because the result of the mul operands may be greater than 16bits
  ; and the result greater than 32.
  %t = call %ov.result.32 @llvm.smul.with.overflow.i32(i32 %rem, i32 %rem)
  %obit = extractvalue %ov.result.32 %t, 1
  ret i1 %obit
}

define %ov.result.32 @ssubtest_reorder(i8 %a) {
  %A = sext i8 %a to i32
  %x = call %ov.result.32 @llvm.ssub.with.overflow.i32(i32 0, i32 %A)
  ret %ov.result.32 %x
; CHECK-LABEL: @ssubtest_reorder
; CHECK: %x = sub nsw i32 0, %A
; CHECK-NEXT: %1 = insertvalue %ov.result.32 { i32 undef, i1 false }, i32 %x, 0
; CHECK-NEXT:  ret %ov.result.32 %1
}

define %ov.result.32 @never_overflows_ssub_test0(i32 %a) {
  %x = call %ov.result.32 @llvm.ssub.with.overflow.i32(i32 %a, i32 0)
  ret %ov.result.32 %x
; CHECK-LABEL: @never_overflows_ssub_test0
; CHECK-NEXT: %[[x:.*]] = insertvalue %ov.result.32 { i32 undef, i1 false }, i32 %a, 0
; CHECK-NEXT:  ret %ov.result.32 %[[x]]
}

define void @cos(double *%P) {
entry:
  %B = tail call double @llvm.cos.f64(double 0.0) nounwind
  store volatile double %B, double* %P

  ret void
; CHECK-LABEL: @cos(
; CHECK: store volatile double 1.000000e+00, double* %P
}

define void @sin(double *%P) {
entry:
  %B = tail call double @llvm.sin.f64(double 0.0) nounwind
  store volatile double %B, double* %P

  ret void
; CHECK-LABEL: @sin(
; CHECK: store volatile double 0.000000e+00, double* %P
}
