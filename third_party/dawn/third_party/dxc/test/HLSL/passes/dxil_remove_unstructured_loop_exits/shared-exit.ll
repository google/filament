; RUN: opt %s -analyze -loops | FileCheck -check-prefix=LOOPBEFORE %s
; RUN: opt %s -dxil-remove-unstructured-loop-exits -o %t.bc
; RUN: opt %t.bc -S | FileCheck %s
; RUN: opt %t.bc -analyze -loops | FileCheck -check-prefix=LOOPAFTER %s

; Two exiting blocks target the same exit block.  This should work.
; Also, the pass should not introduce a new loop, particularly not among
; the latch-exiting blocks.

; LOOPBEFORE:  Loop at depth 1 containing: %header<header>,%then.a<exiting>,%midloop,%then.b<exiting>,%latch<latch><exiting>
; LOOPBEFORE-NOT:  Loop at depth

; Don't create a loop containing: %end<header>,%0<exiting>,%shared_exit<latch>
; LOOPAFTER-NOT:  Loop at depth {{.*}} containing: {{.*}}%end
; LOOPAFTER-NOT:  Loop at depth {{.*}} containing: {{.*}}%0
; LOOPAFTER-NOT:  Loop at depth {{.*}} containing: {{.*}}%shared_exit
; LOOPAFTER:  Loop at depth 1 containing: %header<header>,%then.a,%dx.struct_exit.new_exiting<exiting>,%midloop,%then.b,%dx.struct_exit.new_exiting2<exiting>,%latch<latch><exiting>
; LOOPAFTER-NOT:  Loop at depth

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; The input has two if-then-else structures in a row:
;   header/then.a/midloop
;   midloop/then.b/latch
;
;       entry
;         |
;         v
;   +-> header ---> then.a ------+
;   |     |           |          |
;   |     |   +-------+          |
;   |     v   v                  v
;   |   midloop --> then.b --> shared_exit
;   |     |           |          |
;   |     |   +-------+          |
;   |     v   v                  |
;   +--- latch                   |
;         |                      |
;         | +--------------------+
;         | |
;         v v
;         end


define void @main(i1 %cond) {
entry:
  br label %header

header:
  br i1 %cond, label %then.a, label %midloop

then.a:
  br i1 %cond, label %shared_exit, label %midloop

midloop:
  br i1 %cond, label %then.b, label %latch

then.b:
  br i1 %cond, label %shared_exit, label %latch

latch:
  br i1 %cond, label %end, label %header

shared_exit:
 br label %end

end:
 ret void
}

; CHECK: define void @main
