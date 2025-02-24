//
// main1
//
struct tint_symbol {
  float value : SV_Depth;
};

float main1_inner() {
  return 1.0f;
}

tint_symbol main1() {
  float inner_result = main1_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
//
// main2
//
struct tint_symbol {
  uint value : SV_Coverage;
};

uint main2_inner() {
  return 1u;
}

tint_symbol main2() {
  uint inner_result = main2_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
