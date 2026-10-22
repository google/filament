// RUN: not %dxc -T ps_6_7 -E main -fcgl %s -spirv 2>&1 | FileCheck %s --check-prefix=CHECK-ERROR

vk::SampledTexture2DArray<uint> tex2dArrayuint;

uint4 main() : SV_Target {
    uint status;
// CHECK-ERROR: error: intrinsic 'GatherRaw' method unimplemented
    uint4 val1 = tex2dArrayuint.GatherRaw(float3(0.5, 0.25, 0.1));
// CHECK-ERROR: error: intrinsic 'GatherRaw' method unimplemented
    uint4 val2 = tex2dArrayuint.GatherRaw(float3(0.5, 0.25, 0.1), int2(2, 3));
// CHECK-ERROR: error: intrinsic 'GatherRaw' method unimplemented
    uint4 val3 = tex2dArrayuint.GatherRaw(float3(0.5, 0.25, 0.1), int2(2, 3), status);
    return val1 + val2 + val3;
}
