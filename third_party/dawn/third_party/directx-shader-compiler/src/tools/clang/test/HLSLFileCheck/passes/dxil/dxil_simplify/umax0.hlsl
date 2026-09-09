// RUN: %dxc -Emain -T ps_6_0 %s | FileCheck %s

// Make sure no umax.
// CHECK:%[[a:.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK-NEXT:call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %[[a]])
// CHECK-NEXT:ret void

uint main(uint a:A): SV_Target {
  return max(a, 0);
}