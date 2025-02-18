struct S_atomic {
  int x;
  uint a;
  uint y;
};

struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared S_atomic wg[10];
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if (!((idx < 10u))) {
        break;
      }
      uint x_28 = idx;
      wg[min(x_28, 9u)].x = int(0);
      uint v = 0u;
      InterlockedExchange(wg[min(x_28, 9u)].a, 0u, v);
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
  GroupMemoryBarrierWithGroupSync();
  uint v_1 = 0u;
  InterlockedExchange(wg[4u].a, 1u, v_1);
}

void compute_main_1() {
  uint x_53 = local_invocation_index_1;
  compute_main_inner(x_53);
}

void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    uint v_2 = 0u;
    v_2 = local_invocation_index_1_param;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 10u)) {
        break;
      }
      wg[v_3].x = int(0);
      uint v_4 = 0u;
      InterlockedExchange(wg[v_3].a, 0u, v_4);
      wg[v_3].y = 0u;
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

