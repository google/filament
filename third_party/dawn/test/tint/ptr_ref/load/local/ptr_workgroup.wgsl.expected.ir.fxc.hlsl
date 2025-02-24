struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int i;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    i = int(0);
  }
  GroupMemoryBarrierWithGroupSync();
  i = int(123);
  int u = (i + int(1));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

