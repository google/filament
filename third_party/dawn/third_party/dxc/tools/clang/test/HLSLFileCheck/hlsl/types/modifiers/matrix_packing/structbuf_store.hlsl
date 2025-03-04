// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s | XFail GitHub #1788

// Test writing matrices to structured buffers
// with every combination of source/dest orientations.

typedef row_major int2x2 rmi2x2;
typedef column_major int2x2 cmi2x2;

RWStructuredBuffer<rmi2x2> r;
RWStructuredBuffer<cmi2x2> c;

void main()
{
    // CHECK: i32 11, i32 12, i32 21, i32 22
    r[0] = rmi2x2(11, 12, 21, 22);
    // CHECK: i32 11, i32 12, i32 21, i32 22
    r[1] = cmi2x2(11, 12, 21, 22);
    
    // CHECK: i32 11, i32 21, i32 12, i32 22
    c[0] = rmi2x2(11, 12, 21, 22);
    // CHECK: i32 11, i32 21, i32 12, i32 22
    c[1] = cmi2x2(11, 12, 21, 22);
}