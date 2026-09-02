// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// Note: WaveActiveBitOr() only accepts unsigned interger scalars/vectors.

// CHECK: ; Version: 1.3

struct S {
    uint4 val1;
    uint3 val2;
    uint2 val3;
     uint val4;
};

RWStructuredBuffer<S> values;

// CHECK: OpCapability GroupNonUniformArithmetic

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    uint x = id.x;
    uint4 val1 = values[x].val1;
    uint3 val2 = values[x].val2;
    uint2 val3 = values[x].val3;
     uint val4 = values[x].val4;

// CHECK:      [[val1:%[0-9]+]] = OpLoad %v4uint %val1
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformBitwiseOr %v4uint %uint_3 Reduce [[val1]]
    values[x].val1 = WaveActiveBitOr(val1);
// CHECK:      [[val2:%[0-9]+]] = OpLoad %v3uint %val2
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformBitwiseOr %v3uint %uint_3 Reduce [[val2]]
    values[x].val2 = WaveActiveBitOr(val2);
// CHECK:      [[val3:%[0-9]+]] = OpLoad %v2uint %val3
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformBitwiseOr %v2uint %uint_3 Reduce [[val3]]
    values[x].val3 = WaveActiveBitOr(val3);
// CHECK:      [[val4:%[0-9]+]] = OpLoad %uint %val4
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformBitwiseOr %uint %uint_3 Reduce [[val4]]
    values[x].val4 = WaveActiveBitOr(val4);
}
