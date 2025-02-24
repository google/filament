ByteAddressBuffer S : register(t0);

typedef int S_load_ret[4];
S_load_ret S_load(uint offset) {
  int arr_1[4] = (int[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr_1[i] = asint(S.Load((offset + (i * 4u))));
    }
  }
  return arr_1;
}

typedef int func_S_arr_ret[4];
func_S_arr_ret func_S_arr() {
  return S_load(0u);
}

[numthreads(1, 1, 1)]
void main() {
  int r[4] = func_S_arr();
  return;
}
