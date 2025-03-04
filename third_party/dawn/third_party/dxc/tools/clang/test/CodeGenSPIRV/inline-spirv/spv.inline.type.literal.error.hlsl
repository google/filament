// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

// CHECK: error: The template argument to vk::Literal must be a vk::integral_constant
typedef vk::SpirvOpaqueType</* OpTypeArray */ 28, Texture2D, vk::Literal<bool> > Invalid;

void main() {
  Invalid a;
}
