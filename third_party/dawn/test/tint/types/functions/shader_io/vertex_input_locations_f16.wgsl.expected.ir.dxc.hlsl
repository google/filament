struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  int loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
  float16_t loc4 : TEXCOORD4;
  vector<float16_t, 3> loc5 : TEXCOORD5;
};


float4 main_inner(int loc0, uint loc1, float loc2, float4 loc3, float16_t loc4, vector<float16_t, 3> loc5) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  float4 v = loc3;
  float16_t x = loc4;
  vector<float16_t, 3> y = loc5;
  return (0.0f).xxxx;
}

main_outputs main(main_inputs inputs) {
  main_outputs v_1 = {main_inner(inputs.loc0, inputs.loc1, inputs.loc2, inputs.loc3, inputs.loc4, inputs.loc5)};
  return v_1;
}

