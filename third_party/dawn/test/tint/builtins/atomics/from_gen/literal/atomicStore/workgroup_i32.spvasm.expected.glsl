#version 310 es

uint local_invocation_index_1 = 0u;
shared int arg_0;
void atomicStore_8bea94() {
  atomicExchange(arg_0, 1);
}
void compute_main_inner(uint local_invocation_index_2) {
  atomicExchange(arg_0, 0);
  barrier();
  atomicStore_8bea94();
}
void compute_main_1() {
  uint x_29 = local_invocation_index_1;
  compute_main_inner(x_29);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    atomicExchange(arg_0, 0);
  }
  barrier();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner_1(gl_LocalInvocationIndex);
}
