struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


RWByteAddressBuffer a : register(u0);
[numthreads(16, 1, 1)]
void main() {
  uint value = 42u;
  uint v = 0u;
  a.InterlockedCompareExchange(0u, 0u, value, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 result = {v_1, (v_1 == 0u)};
}

