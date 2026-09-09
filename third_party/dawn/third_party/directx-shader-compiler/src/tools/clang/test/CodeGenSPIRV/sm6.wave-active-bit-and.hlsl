// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// Note: WaveActiveBitAnd() only accepts unsigned interger scalars/vectors.

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
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformBitwiseAnd %v4uint %uint_3 Reduce [[val1]]
    values[x].val1 = WaveActiveBitAnd(val1);
// CHECK:      [[val2:%[0-9]+]] = OpLoad %v3uint %val2
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformBitwiseAnd %v3uint %uint_3 Reduce [[val2]]
    values[x].val2 = WaveActiveBitAnd(val2);
// CHECK:      [[val3:%[0-9]+]] = OpLoad %v2uint %val3
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformBitwiseAnd %v2uint %uint_3 Reduce [[val3]]
    values[x].val3 = WaveActiveBitAnd(val3);
// CHECK:      [[val4:%[0-9]+]] = OpLoad %uint %val4
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformBitwiseAnd %uint %uint_3 Reduce [[val4]]
    values[x].val4 = WaveActiveBitAnd(val4);
}
