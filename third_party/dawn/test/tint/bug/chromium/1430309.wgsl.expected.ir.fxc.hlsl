struct frexp_result_f32 {
  float f;
};

struct frexp_result_f32_1 {
  float fract;
  int exp;
};

struct main_outputs {
  float4 tint_symbol : SV_Target0;
};


static frexp_result_f32 a = (frexp_result_f32)0;
static const frexp_result_f32_1 v = {0.5f, int(1)};
static frexp_result_f32_1 b = v;
float4 main_inner() {
  return float4(a.f, b.fract, 0.0f, 0.0f);
}

main_outputs main() {
  main_outputs v_1 = {main_inner()};
  return v_1;
}

