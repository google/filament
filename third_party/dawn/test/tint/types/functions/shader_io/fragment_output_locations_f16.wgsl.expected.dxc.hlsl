//
// main0
//
struct tint_symbol {
  int value : SV_Target0;
};

int main0_inner() {
  return 1;
}

tint_symbol main0() {
  int inner_result = main0_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
//
// main1
//
struct tint_symbol {
  uint value : SV_Target1;
};

uint main1_inner() {
  return 1u;
}

tint_symbol main1() {
  uint inner_result = main1_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
//
// main2
//
struct tint_symbol {
  float value : SV_Target2;
};

float main2_inner() {
  return 1.0f;
}

tint_symbol main2() {
  float inner_result = main2_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
//
// main3
//
struct tint_symbol {
  float4 value : SV_Target3;
};

float4 main3_inner() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}

tint_symbol main3() {
  float4 inner_result = main3_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
//
// main4
//
struct tint_symbol {
  float16_t value : SV_Target4;
};

float16_t main4_inner() {
  return float16_t(2.25h);
}

tint_symbol main4() {
  float16_t inner_result = main4_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
//
// main5
//
struct tint_symbol {
  vector<float16_t, 3> value : SV_Target5;
};

vector<float16_t, 3> main5_inner() {
  return vector<float16_t, 3>(float16_t(3.0h), float16_t(5.0h), float16_t(8.0h));
}

tint_symbol main5() {
  vector<float16_t, 3> inner_result = main5_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
