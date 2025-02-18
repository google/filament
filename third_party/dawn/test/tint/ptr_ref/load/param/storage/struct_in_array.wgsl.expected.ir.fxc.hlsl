struct str {
  int i;
};


ByteAddressBuffer S : register(t0);
str v(uint offset) {
  str v_1 = {asint(S.Load((offset + 0u)))};
  return v_1;
}

str func(uint pointer_indices[1]) {
  str v_2 = v((0u + (pointer_indices[0u] * 4u)));
  return v_2;
}

[numthreads(1, 1, 1)]
void main() {
  uint v_3[1] = {2u};
  str r = func(v_3);
}

