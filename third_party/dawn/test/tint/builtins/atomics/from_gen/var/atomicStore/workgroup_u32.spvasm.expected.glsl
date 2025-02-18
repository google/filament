#version 310 es

uint local_invocation_index_1 = 0u;
shared uint arg_0;
void atomicStore_726882() {
  uint arg_1 = 0u;
  arg_1 = 1u;
  uint x_18 = arg_1;
  atomicExchange(arg_0, x_18);
}
void compute_main_inner(uint local_invocation_index_2) {
  atomicExchange(arg_0, 0u);
  barrier();
  atomicStore_726882();
}
void compute_main_1() {
  uint x_31 = local_invocation_index_1;
  compute_main_inner(x_31);
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
