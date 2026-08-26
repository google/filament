// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Test reading elements from constant buffer matrices
// with both orientations.

cbuffer cb
{
    row_major int4x4 r;
    column_major int4x4 c;
};
int main() : OUT
{
    // CHECK: %[[r:.+]] = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle {{.*}}, i32 1)
    // CHECK: extractvalue %dx.types.CBufRet.i32 %[[r]], 2
    // CHECK: %[[c:.+]] = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle {{.*}}, i32 6)
    // CHECK: extractvalue %dx.types.CBufRet.i32 %[[c]], 1
    return r._23 + c._23;
}