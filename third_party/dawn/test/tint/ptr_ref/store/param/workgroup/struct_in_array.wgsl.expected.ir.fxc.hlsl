struct str {
  int i;
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared str S[4];
void func(uint pointer_indices[1]) {
  str v = (str)0;
  S[pointer_indices[0u]] = v;
}

void main_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      str v_3 = (str)0;
      S[v_2] = v_3;
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_4[1] = {2u};
  func(v_4);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

