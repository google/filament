#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
shared uint arg_0;
uint atomicOr_5e3d61() {
  uint res = atomicOr(arg_0, 1u);
  return res;
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  v.inner = atomicOr_5e3d61();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
