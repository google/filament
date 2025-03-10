// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

[[vk::builtin("DeviceIndex")]]
float4 main(float a : A) : SV_Target {
    return a.xxxx;
}

// CHECK: :3:3: error: DeviceIndex builtin can only be used as shader input
// CHECK: :3:3: error: DeviceIndex builtin must be of 32-bit scalar integer type
