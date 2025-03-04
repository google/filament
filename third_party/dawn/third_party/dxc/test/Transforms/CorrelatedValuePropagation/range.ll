; RUN: opt -correlated-propagation -S < %s | FileCheck %s

declare i32 @foo()

define i32 @test1(i32 %a) nounwind {
  %a.off = add i32 %a, -8
  %cmp = icmp ult i32 %a.off, 8
  br i1 %cmp, label %then, label %else

then:
  %dead = icmp eq i32 %a, 7
  br i1 %dead, label %end, label %else

else:
  ret i32 1

end:
  ret i32 2

; CHECK-LABEL: @test1(
; CHECK: then:
; CHECK-NEXT: br i1 false, label %end, label %else
}

define i32 @test2(i32 %a) nounwind {
  %a.off = add i32 %a, -8
  %cmp = icmp ult i32 %a.off, 8
  br i1 %cmp, label %then, label %else

then:
  %dead = icmp ugt i32 %a, 15
  br i1 %dead, label %end, label %else

else:
  ret i32 1

end:
  ret i32 2

; CHECK-LABEL: @test2(
; CHECK: then:
; CHECK-NEXT: br i1 false, label %end, label %else
}

; CHECK-LABEL: @test3(
define i32 @test3(i32 %c) nounwind {
  %cmp = icmp slt i32 %c, 2
  br i1 %cmp, label %if.then, label %if.end

if.then:
  ret i32 1

if.end:
  %cmp1 = icmp slt i32 %c, 3
  br i1 %cmp1, label %if.then2, label %if.end8

; CHECK: if.then2
if.then2:
  %cmp2 = icmp eq i32 %c, 2
; CHECK: br i1 true
  br i1 %cmp2, label %if.then4, label %if.end6

; CHECK: if.end6
if.end6:
  ret i32 2

if.then4:
  ret i32 3

if.end8:
  ret i32 4
}

; CHECK-LABEL: @test4(
define i32 @test4(i32 %c) nounwind {
  switch i32 %c, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb
    i32 4, label %sw.bb
  ]

; CHECK: sw.bb
sw.bb:
  %cmp = icmp sge i32 %c, 1
; CHECK: br i1 true
  br i1 %cmp, label %if.then, label %if.end

if.then:
  br label %return

if.end:
  br label %return

sw.default:
  br label %return

return:
  %retval.0 = phi i32 [ 42, %sw.default ], [ 4, %if.then ], [ 9, %if.end ]
  ret i32 %retval.0
}

; CHECK-LABEL: @test5(
define i1 @test5(i32 %c) nounwind {
  %cmp = icmp slt i32 %c, 5
  br i1 %cmp, label %if.then, label %if.end

if.then:
  %cmp1 = icmp eq i32 %c, 4
  br i1 %cmp1, label %if.end, label %if.end8

if.end:
  ret i1 true

if.end8:
  %cmp2 = icmp eq i32 %c, 3
  %cmp3 = icmp eq i32 %c, 4
  %cmp4 = icmp eq i32 %c, 6
; CHECK: %or = or i1 false, false
  %or = or i1 %cmp3, %cmp4
; CHECK: ret i1 %cmp2
  ret i1 %cmp2
}

; CHECK-LABEL: @test6(
define i1 @test6(i32 %c) nounwind {
  %cmp = icmp ule i32 %c, 7
  br i1 %cmp, label %if.then, label %if.end

if.then:
; CHECK: icmp eq i32 %c, 6
; CHECK: br i1
  switch i32 %c, label %if.end [
    i32 6, label %sw.bb
    i32 8, label %sw.bb
  ]

if.end:
  ret i1 true

sw.bb:
  %cmp2 = icmp eq i32 %c, 6
; CHECK: ret i1 true
  ret i1 %cmp2
}

; CHECK-LABEL: @test7(
define i1 @test7(i32 %c) nounwind {
entry:
 switch i32 %c, label %sw.default [
   i32 6, label %sw.bb
   i32 7, label %sw.bb
 ]

sw.bb:
 ret i1 true

sw.default:
 %cmp5 = icmp eq i32 %c, 5
 %cmp6 = icmp eq i32 %c, 6
 %cmp7 = icmp eq i32 %c, 7
 %cmp8 = icmp eq i32 %c, 8
; CHECK: %or = or i1 %cmp5, false
 %or = or i1 %cmp5, %cmp6
; CHECK: %or2 = or i1 false, %cmp8
 %or2 = or i1 %cmp7, %cmp8
 ret i1 false
}
