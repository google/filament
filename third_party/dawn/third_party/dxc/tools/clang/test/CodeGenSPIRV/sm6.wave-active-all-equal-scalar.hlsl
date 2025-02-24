// RUN: %dxc -T cs_6_0 -HV 2018 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

struct S {
    uint val;
    bool res;
};

RWStructuredBuffer<S> values;

// CHECK: OpCapability GroupNonUniformVote

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
// CHECK: [[eq:%[a-zA-Z0-9_]+]] = OpGroupNonUniformAllEqual %bool %uint_3 {{%[a-zA-Z0-9_]+}}
// CHECK: OpSelect %uint [[eq]] %uint_1 %uint_0
    values[id.x].res = WaveActiveAllEqual(values[id.x].val);
}