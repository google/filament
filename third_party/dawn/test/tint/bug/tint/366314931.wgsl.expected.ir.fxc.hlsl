struct S {
  uint3 v;
  uint u;
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared S wgvar;
RWByteAddressBuffer output : register(u0);
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wgvar.v = (0u).xxx;
    uint v_1 = 0u;
    InterlockedExchange(wgvar.u, 0u, v_1);
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_2 = 0u;
  InterlockedOr(wgvar.u, 0u, v_2);
  uint x = v_2;
  uint v_3 = 0u;
  output.InterlockedExchange(12u, x, v_3);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

