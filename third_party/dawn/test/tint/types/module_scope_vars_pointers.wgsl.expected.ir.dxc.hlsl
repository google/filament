struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


static float p = 0.0f;
groupshared float w;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
  float x = (p + w);
  p = x;
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

