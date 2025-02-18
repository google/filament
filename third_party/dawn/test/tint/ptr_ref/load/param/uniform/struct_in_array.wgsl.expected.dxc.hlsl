struct str {
  int4 i;
};

cbuffer cbuffer_S : register(b0) {
  uint4 S[4];
};

str S_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  str tint_symbol = {asint(S[scalar_offset / 4])};
  return tint_symbol;
}

str func_S_X(uint pointer[1]) {
  return S_load((16u * pointer[0]));
}

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1[1] = {2u};
  str r = func_S_X(tint_symbol_1);
  return;
}
