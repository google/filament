struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
RWByteAddressBuffer a : register(u0);

atomic_compare_exchange_result_i32 aatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  a.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


[numthreads(1, 1, 1)]
void compute_main() {
  atomic_compare_exchange_result_i32 tint_symbol = aatomicCompareExchangeWeak(0u, 1, 1);
  int v = tint_symbol.old_value;
  return;
}
