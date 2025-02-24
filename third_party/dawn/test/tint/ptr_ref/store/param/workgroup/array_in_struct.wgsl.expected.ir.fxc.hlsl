struct str {
  int arr[4];
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared str S;
void func() {
  int v[4] = (int[4])0;
  S.arr = v;
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
      S.arr[v_2] = int(0);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  func();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

