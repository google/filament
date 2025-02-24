struct frexp_result_f32_1 {
  float fract;
  int exp;
};
struct frexp_result_f32 {
  float f;
};

static frexp_result_f32 a = (frexp_result_f32)0;
static const frexp_result_f32_1 c = {0.5f, 1};
static frexp_result_f32_1 b = c;

struct tint_symbol {
  float4 value : SV_Target0;
};

float4 main_inner() {
  return float4(a.f, b.fract, 0.0f, 0.0f);
}

tint_symbol main() {
  float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
