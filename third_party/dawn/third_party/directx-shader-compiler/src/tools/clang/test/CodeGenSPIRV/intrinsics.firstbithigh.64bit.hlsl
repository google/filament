// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

void main() {
  uint64_t   uint_1;
  int64_t2   int_2;
// CHECK: error: firstbithigh is currently limited to 32-bit width components when targeting SPIR-V
  int fbh = firstbithigh(uint_1);
// CHECK: error: firstbithigh is currently limited to 32-bit width components when targeting SPIR-V
  int64_t2 fbh2 = firstbithigh(int_2);
}
