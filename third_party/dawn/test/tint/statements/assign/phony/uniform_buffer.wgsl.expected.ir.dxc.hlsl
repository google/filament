struct S {
  int i;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
S v(uint start_byte_offset) {
  S v_1 = {asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)])};
  return v_1;
}

[numthreads(1, 1, 1)]
void main() {
  v(0u);
  asint(u[0u].x);
}

