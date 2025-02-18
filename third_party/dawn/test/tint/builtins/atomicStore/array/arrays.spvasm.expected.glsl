#version 310 es

uint local_invocation_index_1 = 0u;
shared uint wg[3][2][1];
uint tint_mod_u32(uint lhs, uint rhs) {
  return (lhs - ((lhs / mix(rhs, 1u, (rhs == 0u))) * mix(rhs, 1u, (rhs == 0u))));
}
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if (!((idx < 6u))) {
        break;
      }
      uint x_31 = idx;
      uint x_33 = idx;
      uint x_35 = idx;
      uint v = tint_div_u32(x_31, 2u);
      uint v_1 = tint_mod_u32(x_33, 2u);
      uint v_2 = min(tint_mod_u32(x_35, 1u), 0u);
      atomicExchange(wg[min(v, 2u)][min(v_1, 1u)][v_2], 0u);
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
  atomicExchange(wg[2u][1u][0u], 1u);
}
void compute_main_1() {
  uint x_57 = local_invocation_index_1;
  compute_main_inner(x_57);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    uint v_3 = 0u;
    v_3 = local_invocation_index_1_param;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 6u)) {
        break;
      }
      atomicExchange(wg[(v_4 / 2u)][(v_4 % 2u)][0u], 0u);
      {
        v_3 = (v_4 + 1u);
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
