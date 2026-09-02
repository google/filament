// RUN: %dxc -E main -T cs_6_2 -enable-16bit-types %s | FileCheck %s

// Check that compute shader SV_***ID parameters can have 16-bit integer types.
// Regression test for GitHub issue #2268

RWStructuredBuffer<uint3> buf;

[numthreads(1, 1, 1)]
void main(uint16_t3 tid : SV_DispatchThreadID)
{
    // CHECK: call i32 @dx.op.threadId.i32(i32 93, i32 0)
    // CHECK: call i32 @dx.op.threadId.i32(i32 93, i32 1)
    // CHECK: call i32 @dx.op.threadId.i32(i32 93, i32 2)
    // Truncation honors uint16_t type
    // CHECK: and i32 {{.*}}, 65535
    // CHECK: and i32 {{.*}}, 65535
    // CHECK: and i32 {{.*}}, 65535
    // CHECK: call void @dx.op.rawBufferStore.i32
    buf[0] = tid;
}