#version 310 es


struct S0_atomic {
  int x;
  uint a;
  int y;
  int z;
};

struct S1_atomic {
  int x;
  S0_atomic a;
  int y;
  int z;
};

struct S2_atomic {
  int x;
  int y;
  int z;
  S1_atomic a;
};

uint local_invocation_index_1 = 0u;
shared S2_atomic wg;
void compute_main_inner(uint local_invocation_index_2) {
  wg.x = 0;
  wg.y = 0;
  wg.z = 0;
  wg.a.x = 0;
  wg.a.a.x = 0;
  atomicExchange(wg.a.a.a, 0u);
  wg.a.a.y = 0;
  wg.a.a.z = 0;
  wg.a.y = 0;
  wg.a.z = 0;
  barrier();
  atomicExchange(wg.a.a.a, 1u);
}
void compute_main_1() {
  uint x_44 = local_invocation_index_1;
  compute_main_inner(x_44);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    wg.x = 0;
    wg.y = 0;
    wg.z = 0;
    wg.a.x = 0;
    wg.a.a.x = 0;
    atomicExchange(wg.a.a.a, 0u);
    wg.a.a.y = 0;
    wg.a.a.z = 0;
    wg.a.y = 0;
    wg.a.z = 0;
  }
  barrier();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner_1(gl_LocalInvocationIndex);
}
