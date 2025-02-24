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
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  arg_1 = 1u;
  arg_2 = 1u;
  uint x_21 = arg_2;
  uint x_22 = arg_1;
  atomic_compare_exchange_result_u32 tint_symbol = sb_rwatomicCompareExchangeWeak(0u, x_22, x_21);
  uint old_value_1 = tint_symbol.old_value;
  uint x_23 = old_value_1;
  x__atomic_compare_exchange_resultu32 tint_symbol_1 = {x_23, (x_23 == x_21)};
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
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  arg_1 = 1u;
  arg_2 = 1u;
  uint x_21 = arg_2;
  uint x_22 = arg_1;
  atomic_compare_exchange_result_u32 tint_symbol = sb_rwatomicCompareExchangeWeak(0u, x_22, x_21);
  uint old_value_1 = tint_symbol.old_value;
  uint x_23 = old_value_1;
  x__atomic_compare_exchange_resultu32 tint_symbol_1 = {x_23, (x_23 == x_21)};
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
