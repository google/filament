#version 310 es


struct S_atomic {
  int x;
  uint a;
  uint y;
};

uint local_invocation_index_1 = 0u;
shared S_atomic wg[10];
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if (!((idx < 10u))) {
        break;
      }
      uint x_28 = idx;
      wg[min(x_28, 9u)].x = 0;
      atomicExchange(wg[min(x_28, 9u)].a, 0u);
      wg[min(x_28, 9u)].y = 0u;
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        idx = (idx + 1u);
      }
      continue;
    }
  }
  barrier();
  atomicExchange(wg[4u].a, 1u);
}
void compute_main_1() {
  uint x_53 = local_invocation_index_1;
  compute_main_inner(x_53);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    uint v = 0u;
    v = local_invocation_index_1_param;
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
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner_1(gl_LocalInvocationIndex);
}
