struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};
RWByteAddressBuffer a : register(u0);

atomic_compare_exchange_result_u32 aatomicCompareExchangeWeak(uint offset, uint compare, uint value) {
  atomic_compare_exchange_result_u32 result=(atomic_compare_exchange_result_u32)0;
  a.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


[numthreads(16, 1, 1)]
void main() {
  uint value = 42u;
  atomic_compare_exchange_result_u32 result = aatomicCompareExchangeWeak(0u, 0u, value);
  return;
}
