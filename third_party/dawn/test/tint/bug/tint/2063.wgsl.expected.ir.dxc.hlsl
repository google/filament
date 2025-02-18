struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int arg_0;
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    int v = int(0);
    InterlockedExchange(arg_0, int(0), v);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_1 = int(0);
  InterlockedAdd(arg_0, (int(0) - int(-1)), v_1);
  int res = v_1;
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

