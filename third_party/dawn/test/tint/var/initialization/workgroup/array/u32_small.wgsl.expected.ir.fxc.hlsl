struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int zero[3];
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 3u)) {
    zero[tint_local_index] = int(0);
  }
  GroupMemoryBarrierWithGroupSync();
  int v[3] = zero;
}

[numthreads(10, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

