// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv  2>&1 | FileCheck %s

struct foo {
  void method() { }
};

float4 main(float4 color : COLOR) : SV_TARGET {
  return color;
}

// CHECK: warning: Member functions will not be linked to their class in the debug information. Prefer using -fspv-debug=vulkan-with-source. See https://github.com/KhronosGroup/SPIRV-Registry/issues/203
