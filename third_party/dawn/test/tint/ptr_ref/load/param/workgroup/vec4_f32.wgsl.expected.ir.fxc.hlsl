struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared float4 S;
float4 func() {
  return S;
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    S = (0.0f).xxxx;
  }
  GroupMemoryBarrierWithGroupSync();
  float4 r = func();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

