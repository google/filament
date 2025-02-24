#version 310 es


struct S {
  int x;
  uint a[10];
  uint y;
};

shared S wg;
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wg.x = 0;
    wg.y = 0u;
  }
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 10u)) {
        break;
      }
      atomicExchange(wg.a[v_1], 0u);
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  barrier();
  atomicExchange(wg.a[4u], 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
