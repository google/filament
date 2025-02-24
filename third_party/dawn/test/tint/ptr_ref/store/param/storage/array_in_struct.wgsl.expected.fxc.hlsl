RWByteAddressBuffer S : register(u0);

void S_store(uint offset, int value[4]) {
  int array_1[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      S.Store((offset + (i * 4u)), asuint(array_1[i]));
    }
  }
}

void func_S_arr() {
  int tint_symbol[4] = (int[4])0;
  S_store(0u, tint_symbol);
}

[numthreads(1, 1, 1)]
void main() {
  func_S_arr();
  return;
}
