//
// fragment_main
//
struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};
struct x__atomic_compare_exchange_resultu32 {
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
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  atomic_compare_exchange_result_u32 tint_symbol = sb_rwatomicCompareExchangeWeak(0u, 1u, 1u);
  uint old_value_1 = tint_symbol.old_value;
  uint x_17 = old_value_1;
  x__atomic_compare_exchange_resultu32 tint_symbol_1 = {x_17, (x_17 == 1u)};
  res = tint_symbol_1;
  return;
}

void fragment_main_1() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}
//
// compute_main
//
struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};
struct x__atomic_compare_exchange_resultu32 {
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
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  atomic_compare_exchange_result_u32 tint_symbol = sb_rwatomicCompareExchangeWeak(0u, 1u, 1u);
  uint old_value_1 = tint_symbol.old_value;
  uint x_17 = old_value_1;
  x__atomic_compare_exchange_resultu32 tint_symbol_1 = {x_17, (x_17 == 1u)};
  res = tint_symbol_1;
  return;
}

void compute_main_1() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
