struct str {
  int4 i;
};


cbuffer cbuffer_S : register(b0) {
  uint4 S[4];
};
str v(uint start_byte_offset) {
  str v_1 = {asint(S[(start_byte_offset / 16u)])};
  return v_1;
}

str func(uint pointer_indices[1]) {
  str v_2 = v((16u * pointer_indices[0u]));
  return v_2;
}

[numthreads(1, 1, 1)]
void main() {
  uint v_3[1] = {2u};
  str r = func(v_3);
}

