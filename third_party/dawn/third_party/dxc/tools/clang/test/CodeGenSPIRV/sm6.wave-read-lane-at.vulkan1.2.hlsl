// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// CHECK: ; Version: 1.5

struct S {
    float4 val1;
     uint3 val2;
       int val3;
};

RWStructuredBuffer<S> values;

// CHECK: OpCapability GroupNonUniformShuffle

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    uint x = id.x;

    float4 val1 = values[x].val1;
     uint3 val2 = values[x].val2;
       int val3 = values[x].val3;

// CHECK:      [[val1:%[0-9]+]] = OpLoad %v4float %val1
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformShuffle %v4float %uint_3 [[val1]] %uint_15
    values[x].val1 = WaveReadLaneAt(val1, 15);
// CHECK:      [[val2:%[0-9]+]] = OpLoad %v3uint %val2
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformShuffle %v3uint %uint_3 [[val2]] %uint_42
    values[x].val2 = WaveReadLaneAt(val2, 42);
// CHECK:      [[val3:%[0-9]+]] = OpLoad %int %val3
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformShuffle %int %uint_3 [[val3]] %uint_15
    values[x].val3 = WaveReadLaneAt(val3, 15);
}
