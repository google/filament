; RUN: opt %s -analyze -loops | FileCheck -check-prefix=LOOPBEFORE %s
; RUN: opt %s -dxil-remove-unstructured-loop-exits -o %t.bc
; RUN: opt %t.bc -S | FileCheck %s
; RUN: opt %t.bc -analyze -loops | FileCheck -check-prefix=LOOPAFTER %s

; Ensure the pass works when the latch exit is reachable from the exit block,
; and does not introduce an extra loop.
; If an exit block could reach the latch exit block, then an old version
; of the pass would introduce a loop because after modification, the
; latch-exit block would branch to the exit block (which has since been
; repositioned in the CFG).

;
;       entry
;         |
;         v
;   +-> header ---> then --+
;   |     |           |    |
;   |     | +---------+    |
;   |     | |              v
;   |     v v             exit0
;   +--- latch             |
;         |                v
;         |               exit1
;         |                |
;         |                v
;         |               exit2
;         |                |
;         | +--------------+
;         | |
;         v v
;         end     #  'end' is the latch-exit block

; Before performing the loop exit restructuring, split the latch -> end edge, like this:
;
;       entry
;         |
;         v
;   +-> header ---> then --+
;   |     |           |    |
;   |     | +---------+    |
;   |     | |              v
;   |     v v             exit0
;   +--- latch             |
;         |                v
;         |               exit1
;         v                |
;   latch.end_crit_edge    v
;         |               exit2
;         |                |
;         | +--------------+
;         | |
;         v v
;         end

; Then it will be safe to rewire 'then' block as follows.
; This achieves the goal of making all exiting blocks dominate
; the latch.  And crucially, a new loop is not created.
;
;       entry
;         |
;         v
;   +-> header ---> then
;   |     |           |
;   |     | +---------+
;   |     | |
;   |     v v
;   |  dx.struct.new_exiting
;   |     |          |
;   |     v          |
;   +--- latch       |
;         |    +-----+
;         v    v
;   latch.end_crit_edge --> exit0
;         |                  |
;         |                  v
;         |                 exit1
;         |                  |
;         |                  v
;         |                 exit2
;         |                  |
;         | +----------------+
;         | |
;         v v
;         end

; LOOPBEFORE:  Loop at depth 1 containing: %header<header>,%then<exiting>,%latch<latch><exiting>
; LOOPBEFORE-NOT:  Loop at depth

; Don't create an extra loop
; LOOPAFTER-NOT:  Loop at depth {{.*}} containing: {{.*}}%end
; LOOPAFTER-NOT:  Loop at depth {{.*}} containing: {{.*}}%exit0
; LOOPAFTER-NOT:  Loop at depth {{.*}} containing: {{.*}}%exit1
; LOOPAFTER-NOT:  Loop at depth {{.*}} containing: {{.*}}%exit2
; LOOPAFTER:  Loop at depth 1 containing: %header<header>,%then,%dx.struct_exit.new_exiting<exiting>,%latch<latch><exiting>
; LOOPAFTER-NOT:  Loop at depth

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @main(i1 %cond) {
entry:
  br label %header

header:
  br i1 %cond, label %then, label %latch

then:
  br i1 %cond, label %exit0, label %latch

latch:
  br i1 %cond, label %end, label %header

;  a long chain that eventually gets to %end
exit0:
 br label %exit1
exit1:
 br label %exit2
exit2:
 br label %end

end:
 ret void
}

; CHECK: define void @main
