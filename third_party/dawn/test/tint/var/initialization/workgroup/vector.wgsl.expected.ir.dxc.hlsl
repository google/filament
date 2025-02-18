struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int3 v;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v = (int(0)).xxx;
  }
  GroupMemoryBarrierWithGroupSync();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

