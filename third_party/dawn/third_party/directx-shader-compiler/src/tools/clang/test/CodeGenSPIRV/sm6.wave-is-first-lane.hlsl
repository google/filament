// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// CHECK: ; Version: 1.3

RWStructuredBuffer<uint> values;

// CHECK: OpCapability GroupNonUniform

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
// CHECK: {{%[0-9]+}} = OpGroupNonUniformElect %bool %uint_3
    values[id.x] = WaveIsFirstLane();
}
