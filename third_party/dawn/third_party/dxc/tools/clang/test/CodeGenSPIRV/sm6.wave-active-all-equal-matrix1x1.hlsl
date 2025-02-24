// RUN: %dxc -T cs_6_0 -HV 2018 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

struct S {
    float1x1 val;
    bool res;
};

RWStructuredBuffer<S> values;

// CHECK: OpCapability GroupNonUniformVote

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {

// For a 1x1 matrix, the spirv type should become a scalar because Spir-V cannot have a 1x1 matrix.

// CHECK: [[ld:%[a-zA-Z0-9_]+]] = OpLoad %float %32
// CHECK: [[res:%[a-zA-Z0-9_]+]] = OpGroupNonUniformAllEqual %bool %uint_3 [[ld]]
// CHECK: OpSelect %uint [[res]] %uint_1 %uint_0
    values[id.x].res = all(WaveActiveAllEqual(values[id.x].val));
}
