#version 310 es

shared uint wg[4];
void compute_main_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 4u)) {
        break;
      }
      atomicExchange(wg[v_1], 0u);
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  barrier();
  atomicExchange(wg[1u], 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
