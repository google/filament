// RUN: %dxc -E main -T vs_6_0 %s -ftime-trace | FileCheck %s
// RUN: %dxc -E main -T vs_6_0 %s -ftime-trace=%t.json
// RUN: cat %t.json | FileCheck %s

// CHECK: { "traceEvents": [

void main() {}
