Texture2D<float4> t : register(t0);
SamplerState s : register(s0, space1);

struct tint_symbol {
  float4 value : SV_Target0;
};

float4 f_inner() {
  return t.Sample(s, (0.0f).xx, int2(4, 6));
}

tint_symbol f() {
  float4 inner_result = f_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
