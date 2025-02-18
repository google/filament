struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
static bool tint_discarded = false;
RWByteAddressBuffer S : register(u0);

struct tint_symbol_1 {
  float4 value : SV_Target0;
};

atomic_compare_exchange_result_i32 SatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  S.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


float4 main_inner() {
  if (false) {
    tint_discarded = true;
  }
  atomic_compare_exchange_result_i32 tint_symbol_2 = (atomic_compare_exchange_result_i32)0;
  if (!(tint_discarded)) {
    tint_symbol_2 = SatomicCompareExchangeWeak(0u, 0, 1);
  }
  atomic_compare_exchange_result_i32 tint_symbol = tint_symbol_2;
  int old_value = tint_symbol.old_value;
  return float4((float(old_value)).xxxx);
}

tint_symbol_1 main() {
  float4 inner_result = main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return wrapper_result;
}
