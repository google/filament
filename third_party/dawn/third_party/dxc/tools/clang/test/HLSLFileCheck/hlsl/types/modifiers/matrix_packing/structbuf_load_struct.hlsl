// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Test reading matrices from structured buffers
// respects the declared pack orientation.

struct S
{
    row_major int4x4 rm;
    column_major int4x4 cm; // Offset: 64 bytes
};
StructuredBuffer<S> b;

int main() : OUT
{
    S s = b[0];
    // CHECK: %[[row:.*]] = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 0, i32 16)
    // CHECK: extractvalue %dx.types.ResRet.i32 %[[row]], 2
    // CHECK: %[[col:.*]] = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle {{.*}}, i32 0, i32 96)
    // CHECK: extractvalue %dx.types.ResRet.i32 %[[col]], 1
    return s.rm._23 + s.cm._23;
}