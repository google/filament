int tint_symbol() {
  return 0;
}

float tint_symbol_1(int tint_symbol_2) {
  return float(tint_symbol_2);
}

bool tint_symbol_3(float tint_symbol_4) {
  return bool(tint_symbol_4);
}

struct tint_symbol_13 {
  uint tint_symbol_6 : SV_VertexID;
};
struct tint_symbol_14 {
  float4 value : SV_Position;
};

float4 tint_symbol_5_inner(uint tint_symbol_6) {
  float4 tint_symbol_7 = (0.0f).xxxx;
  float4 tint_symbol_8 = (1.0f).xxxx;
  int tint_symbol_9 = tint_symbol();
  float tint_symbol_10 = tint_symbol_1(tint_symbol_9);
  bool tint_symbol_11 = tint_symbol_3(tint_symbol_10);
  return (tint_symbol_11 ? tint_symbol_8 : tint_symbol_7);
}

tint_symbol_14 tint_symbol_5(tint_symbol_13 tint_symbol_12) {
  float4 inner_result = tint_symbol_5_inner(tint_symbol_12.tint_symbol_6);
  tint_symbol_14 wrapper_result = (tint_symbol_14)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
