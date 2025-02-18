#version 310 es


struct S {
  int x;
  uint a;
  uint y;
};

shared S wg[10];
void compute_main_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 10u)) {
        break;
      }
      wg[v_1].x = 0;
      atomicExchange(wg[v_1].a, 0u);
      wg[v_1].y = 0u;
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  barrier();
  atomicExchange(wg[4u].a, 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
