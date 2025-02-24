// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// CHECK: ; Version: 1.3

struct S {
    uint val;
    bool res;
};

RWStructuredBuffer<S> values;

// CHECK: OpCapability GroupNonUniformVote

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    uint x = id.x;
// CHECK:      [[cmp:%[0-9]+]] = OpIEqual %bool {{%[0-9]+}} %uint_1
// CHECK-NEXT:     {{%[0-9]+}} = OpGroupNonUniformAll %bool %uint_3 [[cmp]]
    values[x].res = WaveActiveAllTrue(values[x].val == 1);
}
