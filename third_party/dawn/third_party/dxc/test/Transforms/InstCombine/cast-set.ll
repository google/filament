; This tests for various complex cast elimination cases instcombine should
; handle.

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"

; RUN: opt < %s -instcombine -S | FileCheck %s

define i1 @test1(i32 %X) {
        %A = bitcast i32 %X to i32              ; <i32> [#uses=1]
        ; Convert to setne int %X, 12
        %c = icmp ne i32 %A, 12         ; <i1> [#uses=1]
        ret i1 %c
; CHECK-LABEL @test1(
; CHECK: %c = icmp ne i32 %X, 12
; CHECK: ret i1 %c
}

define i1 @test2(i32 %X, i32 %Y) {
        %A = bitcast i32 %X to i32              ; <i32> [#uses=1]
        %B = bitcast i32 %Y to i32              ; <i32> [#uses=1]
        ; Convert to setne int %X, %Y
        %c = icmp ne i32 %A, %B         ; <i1> [#uses=1]
        ret i1 %c
; CHECK-LABEL @test2(
; CHECK: %c = icmp ne i32 %X, %Y
; CHECK: ret i1 %c
}

define i32 @test4(i32 %A) {
        %B = bitcast i32 %A to i32              ; <i32> [#uses=1]
        %C = shl i32 %B, 2              ; <i32> [#uses=1]
        %D = bitcast i32 %C to i32              ; <i32> [#uses=1]
        ret i32 %D
; CHECK-LABEL: @test4(
; CHECK: %C = shl i32 %A, 2
; CHECK: ret i32 %C
}

define i16 @test5(i16 %A) {
        %B = sext i16 %A to i32         ; <i32> [#uses=1]
        %C = and i32 %B, 15             ; <i32> [#uses=1]
        %D = trunc i32 %C to i16                ; <i16> [#uses=1]
        ret i16 %D
; CHECK-LABEL: @test5(
; CHECK: %C = and i16 %A, 15
; CHECK: ret i16 %C
}

define i1 @test6(i1 %A) {
        %B = zext i1 %A to i32          ; <i32> [#uses=1]
        %C = icmp ne i32 %B, 0          ; <i1> [#uses=1]
        ret i1 %C
; CHECK-LABEL: @test6(
; CHECK: ret i1 %A
}

define i1 @test6a(i1 %A) {
        %B = zext i1 %A to i32          ; <i32> [#uses=1]
        %C = icmp ne i32 %B, -1         ; <i1> [#uses=1]
        ret i1 %C
; CHECK-LABEL: @test6a(
; CHECK: ret i1 true
}

define i1 @test7(i8* %A) {
        %B = bitcast i8* %A to i32*             ; <i32*> [#uses=1]
        %C = icmp eq i32* %B, null              ; <i1> [#uses=1]
        ret i1 %C
; CHECK-LABEL: @test7(
; CHECK: %C = icmp eq i8* %A, null
; CHECK: ret i1 %C
}
