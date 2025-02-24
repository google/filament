struct VSOut {
  float4 pos;
};

void foo(inout VSOut tint_symbol) {
  float4 pos = float4(1.0f, 2.0f, 3.0f, 4.0f);
  tint_symbol.pos = pos;
}

struct tint_symbol_1 {
  float4 pos : SV_Position;
};

VSOut main_inner() {
  VSOut tint_symbol = (VSOut)0;
  foo(tint_symbol);
  return tint_symbol;
}

tint_symbol_1 main() {
  VSOut inner_result = main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
