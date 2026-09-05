// RUN: %dxc -E main -Zi -Od -T vs_6_0 %s | FileCheck %s

// Make sure createHandle has debug info.

// CHECK: @dx.op.createHandle(i32 59, i8 1, i32 0, i32 0, i1 false), !dbg
// CHECK: @dx.op.createHandle(i32 59, i8 1, i32 1, i32 1, i1 false), !dbg

RWBuffer<uint> uav1;
RWBuffer<uint> uav2;

void main()
{
    RWBuffer<uint> u = uav1;
    u[0] = 0;
    u = uav2;
    u[0] = 0;
}
