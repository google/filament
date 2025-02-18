struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared uint wg[4];
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if (!((idx < 4u))) {
        break;
      }
      uint x_26 = idx;
      uint v = 0u;
      InterlockedExchange(wg[min(x_26, 3u)], 0u, v);
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
  GroupMemoryBarrierWithGroupSync();
  uint v_1 = 0u;
  InterlockedExchange(wg[1u], 1u, v_1);
}

void compute_main_1() {
  uint x_47 = local_invocation_index_1;
  compute_main_inner(x_47);
}

void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    uint v_2 = 0u;
    v_2 = local_invocation_index_1_param;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      uint v_4 = 0u;
      InterlockedExchange(wg[v_3], 0u, v_4);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner_1(inputs.local_invocation_index_1_param);
}

