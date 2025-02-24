struct str {
  int i;
};


RWByteAddressBuffer S : register(u0);
void v(uint offset, str obj) {
  S.Store((offset + 0u), asuint(obj.i));
}

void func(uint pointer_indices[1]) {
  str v_1 = (str)0;
  v((0u + (pointer_indices[0u] * 4u)), v_1);
}

[numthreads(1, 1, 1)]
void main() {
  uint v_2[1] = {2u};
  func(v_2);
}

