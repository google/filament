#version 310 es


struct S_atomic {
  int x;
  uint a;
  uint y;
};

uint local_invocation_index_1 = 0u;
shared S_atomic wg;
void compute_main_inner(uint local_invocation_index_2) {
  wg.x = 0;
  atomicExchange(wg.a, 0u);
  wg.y = 0u;
  barrier();
  atomicExchange(wg.a, 1u);
}
void compute_main_1() {
  uint x_35 = local_invocation_index_1;
  compute_main_inner(x_35);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    wg.x = 0;
    atomicExchange(wg.a, 0u);
    wg.y = 0u;
  }
  barrier();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner_1(gl_LocalInvocationIndex);
}
