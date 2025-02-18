struct str {
  int i;
};

RWByteAddressBuffer S : register(u0);

void S_store(uint offset, str value) {
  S.Store((offset + 0u), asuint(value.i));
}

void func_S_X(uint pointer[1]) {
  str tint_symbol = (str)0;
  S_store((4u * pointer[0]), tint_symbol);
}

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1[1] = {2u};
  func_S_X(tint_symbol_1);
  return;
}
