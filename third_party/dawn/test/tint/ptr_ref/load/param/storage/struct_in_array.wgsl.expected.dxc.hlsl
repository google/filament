struct str {
  int i;
};

ByteAddressBuffer S : register(t0);

str S_load(uint offset) {
  str tint_symbol = {asint(S.Load((offset + 0u)))};
  return tint_symbol;
}

str func_S_X(uint pointer[1]) {
  return S_load((4u * pointer[0]));
}

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1[1] = {2u};
  str r = func_S_X(tint_symbol_1);
  return;
}
