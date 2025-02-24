// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

void main() {
  uint64_t   uint_1;
// CHECK: error: firstbitlow is currently limited to 32-bit width components when targeting SPIR-V
  int fbl = firstbitlow(uint_1);
}
