// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv

// This won't work until #5554 is fixed. When it is, add a check that it compiles correctly.

enum class Scope {
  CrossDevice = 0,
  Device = 1,
  Workgroup = 2,
  Subgroup = 3,
  Invocation = 4,
  QueueFamily = 5,
  QueueFamilyKHR = 5,
  ShaderCallKHR = 6,
};

enum class CooperativeMatrixUse {
  MatrixAKHR = 0,
  MatrixBKHR = 1,
  MatrixAccumulatorKHR = 2,
};

typedef vk::SpirvOpaqueType</* OpTypeCooperativeMatrixKHR */ 4456, float, vk::integral_constant<Scope, Scope::Subgroup>, 32, 32, CooperativeMatrixUse::MatrixAKHR> mat_t;

[[vk::ext_extension("SPV_KHR_cooperative_matrix")]]
[[vk::ext_capability(/* CooperativeMatrixKHR */ 6022)]]
void main() {
  mat_t mat;
}
