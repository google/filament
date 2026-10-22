// RUN: %dxc -T cs_6_6 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_6 -E main2 %s | FileCheck %s -check-prefixes=CHECK-MAIN2

// check that entry point attributes are validated even
// in the presence of other entry point functions that are
// not the current entry point function.

// CHECK: @main, !"main", null, null, [[PROPS:![0-9]+]]}
// CHECK: [[PROPS]] = !{i32 4, [[NT:![0-9]+]]}
// CHECK: [[NT]] = !{i32 2, i32 2, i32 1}

// CHECK-MAIN2: @main2, !"main2", null, null, [[PROPS:![0-9]+]]}
// CHECK-MAIN2: [[PROPS]] = !{i32 4, [[NT:![0-9]+]], i32 11, [[WS:![0-9]+]]}
// CHECK-MAIN2: [[NT]] = !{i32 2, i32 2, i32 1}
// CHECK-MAIN2: [[WS]] = !{i32 32}

[wavesize(32)]
[numthreads(2,2,1)]
void main2() {}

[numthreads(2,2,1)]
void main() {}
