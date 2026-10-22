// RUN: %dxc -E main -T vs_6_0 -Od -Zi -fcgl %s | FileCheck %s

// Test that dbg.declares are emitted for temporaries.

int main(int x : IN) : OUT {
  // CHECK: call void @llvm.dbg.declare
  int y = x;
  // CHECK: call void @llvm.dbg.declare
  int z = y;
  return y;
}

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}
