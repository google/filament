// RUN: %dxc -E main -T vs_6_0 %s | FileCheck -check-prefix=FOO %s

// CHECK: this should be ignored
// FOO: main

void main() {}