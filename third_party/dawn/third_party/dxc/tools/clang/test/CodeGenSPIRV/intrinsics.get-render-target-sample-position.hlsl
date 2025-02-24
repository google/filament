// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

void main() {
    uint a = GetRenderTargetSamplePosition(2);
}

// CHECK: :4:14: error: no equivalent for GetRenderTargetSamplePosition intrinsic function in Vulkan
