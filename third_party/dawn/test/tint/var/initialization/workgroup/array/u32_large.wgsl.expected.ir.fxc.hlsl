struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int zero[23];
void main_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 23u)) {
        break;
      }
      zero[v_2] = int(0);
      {
        v_1 = (v_2 + 13u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int v[23] = zero;
}

[numthreads(13, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

