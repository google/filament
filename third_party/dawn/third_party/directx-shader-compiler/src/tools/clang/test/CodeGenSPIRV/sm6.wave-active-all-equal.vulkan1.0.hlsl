// RUN: not %dxc -T cs_6_0 -E main -HV 2018 -fspv-target-env=vulkan1.0 -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
    float4 val1;
    uint val2;
    bool res;
};

RWStructuredBuffer<S> values;

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    uint x = id.x;
    values[x].res = WaveActiveAllEqual(values[x].val1) && WaveActiveAllEqual(values[x].val2);
}

// CHECK: sm6.wave-active-all-equal.vulkan1.0.hlsl:14:21: error: Vulkan 1.1 is required for Wave Operation but not permitted to use
// CHECK-NEXT: values[x].res = WaveActiveAllEqual(values[x].val1) && WaveActiveAllEqual(values[x].val2);
// CHECK: note: please specify your target environment via command line option -fspv-target-env=