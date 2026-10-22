// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -O3  %s -spirv | FileCheck %s

void main() {
}
// CHECK:     OpLine {{%[0-9]+}} 4 1
// CHECK-NOT: OpLine
