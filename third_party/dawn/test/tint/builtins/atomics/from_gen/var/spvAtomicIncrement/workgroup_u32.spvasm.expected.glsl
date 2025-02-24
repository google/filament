#version 310 es

uint local_invocation_index_1 = 0u;
shared uint arg_0;
void atomicAdd_d5db1d() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  uint x_14 = atomicAdd(arg_0, 1u);
  res = x_14;
}
void compute_main_inner(uint local_invocation_index_2) {
  atomicExchange(arg_0, 0u);
  barrier();
  atomicAdd_d5db1d();
}
void compute_main_1() {
  uint x_32 = local_invocation_index_1;
  compute_main_inner(x_32);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner_1(gl_LocalInvocationIndex);
}
