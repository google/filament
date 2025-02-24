// RUN: %dxc -E RayGen -T lib_6_7 %s | FileCheck %s

// Regression test for crashing in DiagnoseRaytracingPayloadAccess
// due to a trivial while(true) loop.

// CHECK: define void @"\01?RayGen@@YAXXZ"()
[shader("raygeneration")] void RayGen() {
    while (true) {
       break;
    }
}
