// RUN: %dxc -E main -T ps_6_6 %s  | FileCheck %s

// Make sure Induction Variable Simplification not widen i32 to i64.
// CHECK:icmp sgt i32
// CHECK-NOT:phi i64
// CHECK:phi i32
// CHECK-NOT:icmp sgt i64
// CHECK:icmp slt i32

struct ST {
  int64_t a;
};

RWStructuredBuffer<ST> u;

int c;

float main() : SV_Target {
  for (int i=0;i<c;i++) {
    InterlockedAdd(u[0].a, i);
  }
  return 0;
}