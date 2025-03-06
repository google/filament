; RUN: opt < %s -instcombine -S | FileCheck %s
; PR1201
define i32 @main(i32 %argc, i8** %argv) {
; CHECK-LABEL: @main(
    %c_19 = alloca i8*
    %malloc_206 = tail call i8* @malloc(i32 mul (i32 ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i32), i32 10))
    store i8* %malloc_206, i8** %c_19
    %tmp_207 = load i8*, i8** %c_19
    tail call void @free(i8* %tmp_207)
    ret i32 0
; CHECK-NEXT: ret i32 0
}

declare noalias i8* @calloc(i32, i32) nounwind
declare noalias i8* @malloc(i32)
declare void @free(i8*)

define i1 @foo() {
; CHECK-LABEL: @foo(
; CHECK-NEXT: ret i1 false
  %m = call i8* @malloc(i32 1)
  %z = icmp eq i8* %m, null
  call void @free(i8* %m)
  ret i1 %z
}

declare void @llvm.lifetime.start(i64, i8*)
declare void @llvm.lifetime.end(i64, i8*)
declare i64 @llvm.objectsize.i64(i8*, i1)
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture, i32, i32, i1) nounwind
declare void @llvm.memmove.p0i8.p0i8.i32(i8* nocapture, i8* nocapture, i32, i32, i1) nounwind
declare void @llvm.memset.p0i8.i32(i8*, i8, i32, i32, i1) nounwind

define void @test3(i8* %src) {
; CHECK-LABEL: @test3(
; CHECK-NEXT: ret void
  %a = call noalias i8* @malloc(i32 10)
  call void @llvm.lifetime.start(i64 10, i8* %a)
  call void @llvm.lifetime.end(i64 10, i8* %a)
  %size = call i64 @llvm.objectsize.i64(i8* %a, i1 true)
  store i8 42, i8* %a
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %a, i8* %src, i32 32, i32 1, i1 false)
  call void @llvm.memmove.p0i8.p0i8.i32(i8* %a, i8* %src, i32 32, i32 1, i1 false)
  call void @llvm.memset.p0i8.i32(i8* %a, i8 5, i32 32, i32 1, i1 false)
  %alloc2 = call noalias i8* @calloc(i32 5, i32 7) nounwind
  %z = icmp ne i8* %alloc2, null
  ret void
}

;; This used to crash.
define void @test4() {
; CHECK-LABEL: @test4(
; CHECK-NEXT: ret void
  %A = call i8* @malloc(i32 16000)
  %B = bitcast i8* %A to double*
  %C = bitcast double* %B to i8*
  call void @free(i8* %C)
  ret void
}

; CHECK-LABEL: @test5(
define void @test5(i8* %ptr, i8** %esc) {
; CHECK-NEXT: call i8* @malloc
; CHECK-NEXT: call i8* @malloc
; CHECK-NEXT: call i8* @malloc
; CHECK-NEXT: call i8* @malloc
; CHECK-NEXT: call i8* @malloc
; CHECK-NEXT: call i8* @malloc
; CHECK-NEXT: call i8* @malloc
; CHECK-NEXT: call void @llvm.memcpy
; CHECK-NEXT: call void @llvm.memmove
; CHECK-NEXT: store
; CHECK-NEXT: call void @llvm.memcpy
; CHECK-NEXT: call void @llvm.memmove
; CHECK-NEXT: call void @llvm.memset
; CHECK-NEXT: store volatile
; CHECK-NEXT: ret
  %a = call i8* @malloc(i32 700)
  %b = call i8* @malloc(i32 700)
  %c = call i8* @malloc(i32 700)
  %d = call i8* @malloc(i32 700)
  %e = call i8* @malloc(i32 700)
  %f = call i8* @malloc(i32 700)
  %g = call i8* @malloc(i32 700)
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %ptr, i8* %a, i32 32, i32 1, i1 false)
  call void @llvm.memmove.p0i8.p0i8.i32(i8* %ptr, i8* %b, i32 32, i32 1, i1 false)
  store i8* %c, i8** %esc
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %d, i8* %ptr, i32 32, i32 1, i1 true)
  call void @llvm.memmove.p0i8.p0i8.i32(i8* %e, i8* %ptr, i32 32, i32 1, i1 true)
  call void @llvm.memset.p0i8.i32(i8* %f, i8 5, i32 32, i32 1, i1 true)
  store volatile i8 4, i8* %g
  ret void
}

;; When a basic block contains only a call to free and this block is accessed
;; through a test of the argument of free against null, move the call in the
;; predecessor block.
;; Using simplifycfg will remove the empty basic block and the branch operation
;; Then, performing a dead elimination will remove the comparison.
;; This is what happens with -O1 and upper.
; CHECK-LABEL: @test6(
define void @test6(i8* %foo) minsize {
; CHECK:  %tobool = icmp eq i8* %foo, null
;; Call to free moved
; CHECK-NEXT: tail call void @free(i8* %foo)
; CHECK-NEXT: br i1 %tobool, label %if.end, label %if.then
; CHECK: if.then:
;; Block is now empty and may be simplified by simplifycfg
; CHECK-NEXT:   br label %if.end
; CHECK: if.end:
; CHECK-NEXT:  ret void
entry:
  %tobool = icmp eq i8* %foo, null
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @free(i8* %foo)
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  ret void
}

declare i8* @_ZnwmRKSt9nothrow_t(i64, i8*) nobuiltin
declare void @_ZdlPvRKSt9nothrow_t(i8*, i8*) nobuiltin
declare i32 @__gxx_personality_v0(...)
declare void @_ZN1AC2Ev(i8* %this)

; CHECK-LABEL: @test7(
define void @test7() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %nt = alloca i8
  ; CHECK-NOT: call {{.*}}@_ZnwmRKSt9nothrow_t(
  %call.i = tail call i8* @_ZnwmRKSt9nothrow_t(i64 1, i8* %nt) builtin nounwind
  invoke void @_ZN1AC2Ev(i8* undef)
          to label %.noexc.i unwind label %lpad.i

.noexc.i:                                         ; preds = %entry
  unreachable

lpad.i:                                           ; preds = %entry
  %0 = landingpad { i8*, i32 } cleanup
  ; CHECK-NOT: call {{.*}}@_ZdlPvRKSt9nothrow_t(
  call void @_ZdlPvRKSt9nothrow_t(i8* %call.i, i8* %nt) builtin nounwind
  resume { i8*, i32 } %0
}

declare i8* @_Znwm(i64) nobuiltin
declare i8* @_Znwj(i32) nobuiltin
declare i8* @_Znam(i64) nobuiltin
declare i8* @_Znaj(i32) nobuiltin
declare void @_ZdlPv(i8*) nobuiltin
declare void @_ZdaPv(i8*) nobuiltin

define linkonce void @_ZdlPvm(i8* %p, i64) nobuiltin {
  call void @_ZdlPv(i8* %p)
  ret void
}
define linkonce void @_ZdlPvj(i8* %p, i32) nobuiltin {
  call void @_ZdlPv(i8* %p)
  ret void
}
define linkonce void @_ZdaPvm(i8* %p, i64) nobuiltin {
  call void @_ZdaPv(i8* %p)
  ret void
}
define linkonce void @_ZdaPvj(i8* %p, i32) nobuiltin {
  call void @_ZdaPv(i8* %p)
  ret void
}

; CHECK-LABEL: @test8(
define void @test8() {
  ; CHECK-NOT: call
  %nw = call i8* @_Znwm(i64 32) builtin
  call void @_ZdlPv(i8* %nw) builtin
  %na = call i8* @_Znam(i64 32) builtin
  call void @_ZdaPv(i8* %na) builtin
  %nwm = call i8* @_Znwm(i64 32) builtin
  call void @_ZdlPvm(i8* %nwm, i64 32) builtin
  %nwj = call i8* @_Znwj(i32 32) builtin
  call void @_ZdlPvj(i8* %nwj, i32 32) builtin
  %nam = call i8* @_Znam(i64 32) builtin
  call void @_ZdaPvm(i8* %nam, i64 32) builtin
  %naj = call i8* @_Znaj(i32 32) builtin
  call void @_ZdaPvj(i8* %naj, i32 32) builtin
  ret void
}
