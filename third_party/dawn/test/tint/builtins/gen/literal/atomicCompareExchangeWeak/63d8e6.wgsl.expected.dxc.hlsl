//
// fragment_main
//
struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};
RWByteAddressBuffer sb_rw : register(u0);

atomic_compare_exchange_result_u32 sb_rwatomicCompareExchangeWeak(uint offset, uint compare, uint value) {
  atomic_compare_exchange_result_u32 result=(atomic_compare_exchange_result_u32)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_63d8e6() {
  atomic_compare_exchange_result_u32 res = sb_rwatomicCompareExchangeWeak(0u, 1u, 1u);
}

void fragment_main() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}
//
// compute_main
//
struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};
RWByteAddressBuffer sb_rw : register(u0);

atomic_compare_exchange_result_u32 sb_rwatomicCompareExchangeWeak(uint offset, uint compare, uint value) {
  atomic_compare_exchange_result_u32 result=(atomic_compare_exchange_result_u32)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_63d8e6() {
  atomic_compare_exchange_result_u32 res = sb_rwatomicCompareExchangeWeak(0u, 1u, 1u);
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}
