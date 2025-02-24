struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
static bool tint_discarded = false;
RWByteAddressBuffer a : register(u0);

struct tint_symbol {
  int value : SV_Target0;
};

atomic_compare_exchange_result_i32 aatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  a.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


int foo_inner() {
  tint_discarded = true;
  int x = 0;
  atomic_compare_exchange_result_i32 tint_symbol_1 = (atomic_compare_exchange_result_i32)0;
  if (!(tint_discarded)) {
    tint_symbol_1 = aatomicCompareExchangeWeak(0u, 0, 1);
  }
  atomic_compare_exchange_result_i32 result = tint_symbol_1;
  if (result.exchanged) {
    x = result.old_value;
  }
  return x;
}

tint_symbol foo() {
  int inner_result = foo_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return wrapper_result;
}
