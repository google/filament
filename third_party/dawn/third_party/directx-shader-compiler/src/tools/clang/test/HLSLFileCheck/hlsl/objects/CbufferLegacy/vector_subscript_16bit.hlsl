// RUN: %dxc -E main -T ps_6_2 -enable-16bit-types -Od %s | FileCheck %s

cbuffer constants : register(b0)
{
    half4 foo;
}

float main() : SV_TARGET
{
    float ret = 0;
    uint index = 9;
    if (index < 8) {
        ret += foo[index];
    }
    return ret;
}

// Regression test for Od when an out of bound access happens in a dead block.
// We want to make sure it doesn't crash, compiles successfully, and the
// cbuffer load doesn't actually get generated.

// CHECK: @main() {
// CHECK-NOT: @dx.op.cbufferLoadLegacy.f32(i32 59
// CHECK: call void @dx.op.storeOutput.f32(
