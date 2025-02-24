struct S {
  int x;
  uint a;
  uint y;
};

struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared S wg;
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wg.x = int(0);
    uint v = 0u;
    InterlockedExchange(wg.a, 0u, v);
    wg.y = 0u;
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_1 = 0u;
  InterlockedExchange(wg.a, 1u, v_1);
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

