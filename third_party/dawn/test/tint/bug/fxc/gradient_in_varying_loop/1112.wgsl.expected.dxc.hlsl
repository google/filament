SamplerState tint_symbol : register(s0);
Texture2D<float4> randomTexture : register(t1);

struct tint_symbol_3 {
  float2 vUV : TEXCOORD0;
};
struct tint_symbol_4 {
  float4 value : SV_Target0;
};

float4 main_inner(float2 vUV) {
  float4 tint_symbol_1 = randomTexture.Sample(tint_symbol, vUV);
  float3 random = tint_symbol_1.rgb;
  int i = 0;
  while (true) {
    if ((i < 1)) {
    } else {
      break;
    }
    float3 offset = float3((random.x).xxx);
    bool tint_tmp_2 = (offset.x < 0.0f);
    if (!tint_tmp_2) {
      tint_tmp_2 = (offset.y < 0.0f);
    }
    bool tint_tmp_1 = (tint_tmp_2);
    if (!tint_tmp_1) {
      tint_tmp_1 = (offset.x > 1.0f);
    }
    bool tint_tmp = (tint_tmp_1);
    if (!tint_tmp) {
      tint_tmp = (offset.y > 1.0f);
    }
    if ((tint_tmp)) {
      i = (i + 1);
      continue;
    }
    float sampleDepth = 0.0f;
    i = (i + 1);
  }
  return (1.0f).xxxx;
}

tint_symbol_4 main(tint_symbol_3 tint_symbol_2) {
  float4 inner_result = main_inner(tint_symbol_2.vUV);
  tint_symbol_4 wrapper_result = (tint_symbol_4)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
