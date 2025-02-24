//
// fragment_main
//
struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
RWByteAddressBuffer sb_rw : register(u0);

atomic_compare_exchange_result_i32 sb_rwatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_1bd40a() {
  atomic_compare_exchange_result_i32 res = sb_rwatomicCompareExchangeWeak(0u, 1, 1);
}

void fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}
//
// compute_main
//
struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
RWByteAddressBuffer sb_rw : register(u0);

atomic_compare_exchange_result_i32 sb_rwatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_1bd40a() {
  atomic_compare_exchange_result_i32 res = sb_rwatomicCompareExchangeWeak(0u, 1, 1);
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}
