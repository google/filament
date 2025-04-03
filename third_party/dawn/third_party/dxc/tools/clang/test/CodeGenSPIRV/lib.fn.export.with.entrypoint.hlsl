// RUN: %dxc -T lib_6_6 -E main -fspv-target-env=universal1.5 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint MissKHR %miss "miss" %payload
// CHECK: OpDecorate %func LinkageAttributes "func" Export


struct RayPayload
{
    uint a;
};

export void func()
{
}

[shader("miss")]
void miss(inout RayPayload payload)
{
}
