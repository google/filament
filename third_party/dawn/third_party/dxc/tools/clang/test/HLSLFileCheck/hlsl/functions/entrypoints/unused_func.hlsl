// RUN: %dxc -T vs_6_1 -fcgl %s | FileCheck %s

// Make sure unused functions are not generated.

// CHECK-NOT: unused_function_name

void unused_function_name() {}
void main() {}

