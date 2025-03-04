// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Check that compute shader SV_***ID parameters can have min-integer types.
// Regression test for GitHub issue #2268

RWStructuredBuffer<uint3> buf;

[numthreads(1, 1, 1)]
void main(min12int3 tid : SV_DispatchThreadID)
{
    // CHECK: call i32 @dx.op.threadId.i32(i32 93, i32 0)
    // CHECK: call i32 @dx.op.threadId.i32(i32 93, i32 1)
    // CHECK: call i32 @dx.op.threadId.i32(i32 93, i32 2)
    // Truncation honors uint16_t type
    // CHECK: shl i32 %{{.*}}, 16
    // CHECK: ashr exact i32 %{{.*}}, 16
    // CHECK: shl i32 %{{.*}}, 16
    // CHECK: ashr exact i32 %{{.*}}, 16
    // CHECK: shl i32 %{{.*}}, 16
    // CHECK: ashr exact i32 %{{.*}}, 16
    // CHECK: call void @dx.op.bufferStore.i32
    buf[0] = tid;
}