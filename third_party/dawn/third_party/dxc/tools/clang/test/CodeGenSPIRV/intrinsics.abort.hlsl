// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

void main() {
    abort();
}

// CHECK: :4:5: error: no equivalent for abort intrinsic function in Vulkan
