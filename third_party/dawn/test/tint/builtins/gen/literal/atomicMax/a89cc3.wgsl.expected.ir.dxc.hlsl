struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared int arg_0;
int atomicMax_a89cc3() {
  int v = int(0);
  InterlockedMax(arg_0, int(1), v);
  int res = v;
  return res;
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    int v_1 = int(0);
    InterlockedExchange(arg_0, int(0), v_1);
  }
  GroupMemoryBarrierWithGroupSync();
  prevent_dce.Store(0u, asuint(atomicMax_a89cc3()));
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

