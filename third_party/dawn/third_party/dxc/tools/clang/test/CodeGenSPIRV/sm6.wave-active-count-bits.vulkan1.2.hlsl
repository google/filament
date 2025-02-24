// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// CHECK: ; Version: 1.5

struct S {
     uint val;
};

RWStructuredBuffer<S> values;
RWStructuredBuffer<S> results;

// CHECK: OpCapability GroupNonUniformBallot

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    uint x = id.x;

// CHECK:         [[cmp:%[0-9]+]] = OpIEqual %bool {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[ballot:%[0-9]+]] = OpGroupNonUniformBallot %v4uint %uint_3 [[cmp]]
// CHECK:             {{%[0-9]+}} = OpGroupNonUniformBallotBitCount %uint %uint_3 Reduce [[ballot]]
    results[x].val = WaveActiveCountBits(values[x].val == 0);
}

