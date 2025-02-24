// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// CHECK: ; Version: 1.3

struct S {
     int4 val1;
    uint3 val2;
    float val3;
};

RWStructuredBuffer<S> values;

// CHECK: OpCapability GroupNonUniformQuad

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    uint x = id.x;

     int4 val1 = values[x].val1;
    uint3 val2 = values[x].val2;
    float val3 = values[x].val3;

// CHECK:      [[val1:%[0-9]+]] = OpLoad %v4int %val1
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformQuadSwap %v4int %uint_3 [[val1]] %uint_1
    values[x].val1 = QuadReadAcrossY(val1);
// CHECK:      [[val2:%[0-9]+]] = OpLoad %v3uint %val2
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformQuadSwap %v3uint %uint_3 [[val2]] %uint_1
    values[x].val2 = QuadReadAcrossY(val2);
// CHECK:      [[val3:%[0-9]+]] = OpLoad %float %val3
// CHECK-NEXT:      {{%[0-9]+}} = OpGroupNonUniformQuadSwap %float %uint_3 [[val3]] %uint_1
    values[x].val3 = QuadReadAcrossY(val3);
}
