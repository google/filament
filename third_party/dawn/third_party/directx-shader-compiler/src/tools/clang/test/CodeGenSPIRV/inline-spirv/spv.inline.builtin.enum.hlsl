// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv -verify

// expected-no-diagnostics

enum class BuiltIn {
  HelperInvocation = 23
};

[[vk::ext_builtin_input(BuiltIn::HelperInvocation)]]
static const bool gl_HelperInvocation;

uint square_x(uint3 v) {
  return v.x * v.x;
}

[numthreads(32,1,1)]
void main() {
}
