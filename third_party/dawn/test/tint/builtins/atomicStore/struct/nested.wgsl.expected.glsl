#version 310 es


struct S0 {
  int x;
  uint a;
  int y;
  int z;
};

struct S1 {
  int x;
  S0 a;
  int y;
  int z;
};

struct S2 {
  int x;
  int y;
  int z;
  S1 a;
};

shared S2 wg;
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
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
  atomicExchange(wg.a.a.a, 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
