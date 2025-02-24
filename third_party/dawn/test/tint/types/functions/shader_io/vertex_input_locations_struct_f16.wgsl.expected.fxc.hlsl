SKIP: INVALID

struct VertexInputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float16_t loc4;
  vector<float16_t, 3> loc5;
};
struct tint_symbol_1 {
  int loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
  float16_t loc4 : TEXCOORD4;
  vector<float16_t, 3> loc5 : TEXCOORD5;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 main_inner(VertexInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  float4 v = inputs.loc3;
  float16_t x = inputs.loc4;
  vector<float16_t, 3> y = inputs.loc5;
  return (0.0f).xxxx;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  VertexInputs tint_symbol_3 = {tint_symbol.loc0, tint_symbol.loc1, tint_symbol.loc2, tint_symbol.loc3, tint_symbol.loc4, tint_symbol.loc5};
  float4 inner_result = main_inner(tint_symbol_3);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(6,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
