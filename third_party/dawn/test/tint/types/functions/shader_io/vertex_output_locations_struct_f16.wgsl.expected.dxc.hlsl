struct VertexOutputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float4 position;
  float16_t loc4;
  vector<float16_t, 3> loc5;
};
struct tint_symbol {
  nointerpolation int loc0 : TEXCOORD0;
  nointerpolation uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
  float16_t loc4 : TEXCOORD4;
  vector<float16_t, 3> loc5 : TEXCOORD5;
  float4 position : SV_Position;
};

VertexOutputs main_inner() {
  VertexOutputs tint_symbol_1 = {1, 1u, 1.0f, float4(1.0f, 2.0f, 3.0f, 4.0f), (0.0f).xxxx, float16_t(2.25h), vector<float16_t, 3>(float16_t(3.0h), float16_t(5.0h), float16_t(8.0h))};
  return tint_symbol_1;
}

tint_symbol main() {
  VertexOutputs inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.loc0 = inner_result.loc0;
  wrapper_result.loc1 = inner_result.loc1;
  wrapper_result.loc2 = inner_result.loc2;
  wrapper_result.loc3 = inner_result.loc3;
  wrapper_result.position = inner_result.position;
  wrapper_result.loc4 = inner_result.loc4;
  wrapper_result.loc5 = inner_result.loc5;
  return wrapper_result;
}
