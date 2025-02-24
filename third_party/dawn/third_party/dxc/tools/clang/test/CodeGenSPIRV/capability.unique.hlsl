// RUN: %dxc -T ps_6_2 -E main -fcgl  %s -spirv | FileCheck %s

// Make sure the same capability is not applied twice.
//
// CHECK:     OpCapability Int64
// CHECK-NOT: OpCapability Int64

void main() {
  int64_t a = 1;
  int64_t b = 2;
  int64_t c = a + b;
}

