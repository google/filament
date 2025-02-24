void bar() {
}

struct tint_symbol {
  float4 value : SV_Target0;
};

float4 main_inner() {
  float2 a = (0.0f).xx;
  bar();
  return float4(0.40000000596046447754f, 0.40000000596046447754f, 0.80000001192092895508f, 1.0f);
}

tint_symbol main() {
  float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
