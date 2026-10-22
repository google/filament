// RUN: %dxc -E main -T vs_6_0 %s -ftime-report | FileCheck %s

// CHECK:      ; ===-----------------------------------------
// CHECK-NEXT: ;                       ... Pass execution timing report ...
// CHECK-NEXT: ; ===-----------------------------------------

void main() {}
