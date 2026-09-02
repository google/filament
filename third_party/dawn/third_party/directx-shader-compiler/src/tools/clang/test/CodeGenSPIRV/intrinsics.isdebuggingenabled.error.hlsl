// RUN: not %dxc -T cs_6_10 -spirv %s 2>&1 | FileCheck %s

// CHECK: :9:9: error: no equivalent for IsDebuggingEnabled intrinsic function in Vulkan

RWStructuredBuffer<uint> Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID) {
    if (dx::IsDebuggingEnabled()) {
        Output[threadId.x] = 1;
    }
}
