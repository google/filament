struct S {
  int x;
  uint a[10];
  uint y;
};

struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared S wg;
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wg.x = int(0);
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
      uint v_2 = 0u;
      InterlockedExchange(wg.a[v_1], 0u, v_2);
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_3 = 0u;
  InterlockedExchange(wg.a[4u], 1u, v_3);
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

