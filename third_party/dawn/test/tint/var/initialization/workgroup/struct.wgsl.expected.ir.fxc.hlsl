struct S {
  int a;
  float b;
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared S v;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    S v_1 = (S)0;
    v = v_1;
  }
  GroupMemoryBarrierWithGroupSync();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

