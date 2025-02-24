// RUN: %dxc -T lib_6_4 %s -Wno-parameter-usage | FileCheck %s

// FIXME: This should be a `-verify` test but can't be until we move to lit

// CHECK-NOT: warning
// CHECK: target datalayout

void UnusedEmpty(out int Val) {}

int ReturnedMaybePassthrough(int Cond, out int Val) {
  if (Cond % 3)
    UnusedEmpty(Val);
  else if (Cond % 2)
    UnusedEmpty(Val);
  return Val;
}
