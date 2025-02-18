struct str {
  float4 i;
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared str S;
float4 func() {
  return S.i;
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    str v = (str)0;
    S = v;
  }
  GroupMemoryBarrierWithGroupSync();
  float4 r = func();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

