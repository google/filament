// RUN: %dxc -E main -T ps_6_0 -Od %s | FileCheck %s

const float4 foo;

float main() : SV_TARGET
{
    float ret = 0;
    uint index = 5;
    if (index < 10) {
        ret += foo[index];
    }
    return ret;
}

// Regression test for Od when an out of bound access happens but:
// 1) the front-end can't immediate figure it out,
// 2) when lowering the cbuffer load, the index is resolved to be a constant
// 3) lowering code crashed because it assumed OOB access isn't possible when the index is constant.

// CHECK: 10:{{[0-9]+}}: error: Out of bounds index (5) in CBuffer '$Globals'


