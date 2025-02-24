struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint wg[3][2][1];
void compute_main_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 6u)) {
        break;
      }
      uint v_2 = 0u;
      InterlockedExchange(wg[(v_1 / 2u)][(v_1 % 2u)][0u], 0u, v_2);
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_3 = 0u;
  InterlockedExchange(wg[2u][1u][0u], 1u, v_3);
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

