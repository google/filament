float tint_trunc(float param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

cbuffer cbuffer_x_20 : register(b9, space2) {
  uint4 x_20[8];
};
Texture2D<float4> frameMapTexture : register(t3, space2);
SamplerState frameMapSampler : register(s2, space2);
static float2 tUV = float2(0.0f, 0.0f);
Texture2D<float4> tileMapsTexture0 : register(t5, space2);
SamplerState tileMapsSampler : register(s4, space2);
Texture2D<float4> tileMapsTexture1 : register(t6, space2);
Texture2D<float4> animationMapTexture : register(t8, space2);
SamplerState animationMapSampler : register(s7, space2);
static float mt = 0.0f;
Texture2D<float4> spriteSheetTexture : register(t1, space2);
SamplerState spriteSheetSampler : register(s0, space2);
static float4 glFragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float2 tileID_1 = float2(0.0f, 0.0f);
static float2 levelUnits = float2(0.0f, 0.0f);
static float2 stageUnits_1 = float2(0.0f, 0.0f);
static float3 vPosition = float3(0.0f, 0.0f, 0.0f);
static float2 vUV = float2(0.0f, 0.0f);

float4x4 getFrameData_f1_(inout float frameID) {
  float fX = 0.0f;
  float x_15 = frameID;
  float x_25 = asfloat(x_20[6].w);
  fX = (x_15 / x_25);
  float x_37 = fX;
  float4 x_40 = frameMapTexture.SampleBias(frameMapSampler, float2(x_37, 0.0f), clamp(0.0f, -16.0f, 15.99f));
  float x_44 = fX;
  float4 x_47 = frameMapTexture.SampleBias(frameMapSampler, float2(x_44, 0.25f), clamp(0.0f, -16.0f, 15.99f));
  float x_51 = fX;
  float4 x_54 = frameMapTexture.SampleBias(frameMapSampler, float2(x_51, 0.5f), clamp(0.0f, -16.0f, 15.99f));
  return float4x4(float4(x_40.x, x_40.y, x_40.z, x_40.w), float4(x_47.x, x_47.y, x_47.z, x_47.w), float4(x_54.x, x_54.y, x_54.z, x_54.w), (0.0f).xxxx);
}

float tint_float_mod(float lhs, float rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

void main_1() {
  float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2 tileUV = float2(0.0f, 0.0f);
  float2 tileID = float2(0.0f, 0.0f);
  float2 sheetUnits = float2(0.0f, 0.0f);
  float spriteUnits = 0.0f;
  float2 stageUnits = float2(0.0f, 0.0f);
  int i = 0;
  float frameID_1 = 0.0f;
  float4 animationData = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float f = 0.0f;
  float4x4 frameData = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float param = 0.0f;
  float2 frameSize = float2(0.0f, 0.0f);
  float2 offset_1 = float2(0.0f, 0.0f);
  float2 ratio = float2(0.0f, 0.0f);
  float4 nc = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float alpha = 0.0f;
  float3 mixed = float3(0.0f, 0.0f, 0.0f);
  color = (0.0f).xxxx;
  float2 x_86 = tUV;
  tileUV = frac(x_86);
  float x_91 = tileUV.y;
  tileUV.y = (1.0f - x_91);
  float2 x_95 = tUV;
  tileID = floor(x_95);
  float2 x_101 = asfloat(x_20[6].xy);
  sheetUnits = ((1.0f).xx / x_101);
  float x_106 = asfloat(x_20[6].w);
  spriteUnits = (1.0f / x_106);
  float2 x_111 = asfloat(x_20[5].zw);
  stageUnits = ((1.0f).xx / x_111);
  i = 0;
  while (true) {
    int x_122 = i;
    if ((x_122 < 2)) {
    } else {
      break;
    }
    int x_126 = i;
    switch(x_126) {
      case 1: {
        float2 x_150 = tileID;
        float2 x_154 = asfloat(x_20[5].zw);
        float4 x_156 = tileMapsTexture1.SampleBias(tileMapsSampler, ((x_150 + (0.5f).xx) / x_154), clamp(0.0f, -16.0f, 15.99f));
        frameID_1 = x_156.x;
        break;
      }
      case 0: {
        float2 x_136 = tileID;
        float2 x_140 = asfloat(x_20[5].zw);
        float4 x_142 = tileMapsTexture0.SampleBias(tileMapsSampler, ((x_136 + (0.5f).xx) / x_140), clamp(0.0f, -16.0f, 15.99f));
        frameID_1 = x_142.x;
        break;
      }
      default: {
        break;
      }
    }
    float x_166 = frameID_1;
    float x_169 = asfloat(x_20[6].w);
    float4 x_172 = animationMapTexture.SampleBias(animationMapSampler, float2(((x_166 + 0.5f) / x_169), 0.0f), clamp(0.0f, -16.0f, 15.99f));
    animationData = x_172;
    float x_174 = animationData.y;
    if ((x_174 > 0.0f)) {
      float x_181 = asfloat(x_20[0].x);
      float x_184 = animationData.z;
      mt = tint_float_mod((x_181 * x_184), 1.0f);
      f = 0.0f;
      while (true) {
        float x_193 = f;
        if ((x_193 < 8.0f)) {
        } else {
          break;
        }
        float x_197 = animationData.y;
        float x_198 = mt;
        if ((x_197 > x_198)) {
          float x_203 = animationData.x;
          frameID_1 = x_203;
          break;
        }
        float x_208 = frameID_1;
        float x_211 = asfloat(x_20[6].w);
        float x_214 = f;
        float4 x_217 = (0.0f).xxxx;
        animationData = x_217;
        {
          float x_218 = f;
          f = (x_218 + 1.0f);
        }
      }
    }
    float x_222 = frameID_1;
    param = (x_222 + 0.5f);
    float4x4 x_225 = getFrameData_f1_(param);
    frameData = x_225;
    float4 x_228 = frameData[0];
    float2 x_231 = asfloat(x_20[6].xy);
    frameSize = (float2(x_228.w, x_228.z) / x_231);
    float4 x_235 = frameData[0];
    float2 x_237 = sheetUnits;
    offset_1 = (float2(x_235.x, x_235.y) * x_237);
    float4 x_241 = frameData[2];
    float4 x_244 = frameData[0];
    ratio = (float2(x_241.x, x_241.y) / float2(x_244.w, x_244.z));
    float x_248 = frameData[2].z;
    if ((x_248 == 1.0f)) {
      float2 x_252 = tileUV;
      tileUV = float2(x_252.y, x_252.x);
    }
    int x_254 = i;
    if ((x_254 == 0)) {
      float2 x_263 = tileUV;
      float2 x_264 = frameSize;
      float2 x_266 = offset_1;
      float4 x_268 = spriteSheetTexture.Sample(spriteSheetSampler, ((x_263 * x_264) + x_266));
      color = x_268;
    } else {
      float2 x_274 = tileUV;
      float2 x_275 = frameSize;
      float2 x_277 = offset_1;
      float4 x_279 = spriteSheetTexture.Sample(spriteSheetSampler, ((x_274 * x_275) + x_277));
      nc = x_279;
      float x_283 = color.w;
      float x_285 = nc.w;
      alpha = min((x_283 + x_285), 1.0f);
      float4 x_290 = color;
      float4 x_292 = nc;
      float x_295 = nc.w;
      mixed = lerp(float3(x_290.x, x_290.y, x_290.z), float3(x_292.x, x_292.y, x_292.z), float3(x_295, x_295, x_295));
      float3 x_298 = mixed;
      float x_299 = alpha;
      color = float4(x_298.x, x_298.y, x_298.z, x_299);
    }
    {
      int x_304 = i;
      i = (x_304 + 1);
    }
  }
  float3 x_310 = asfloat(x_20[7].xyz);
  float4 x_311 = color;
  float3 x_313 = (float3(x_311.x, x_311.y, x_311.z) * x_310);
  float4 x_314 = color;
  color = float4(x_313.x, x_313.y, x_313.z, x_314.w);
  float4 x_318 = color;
  glFragColor = x_318;
  return;
}

struct main_out {
  float4 glFragColor_1;
};
struct tint_symbol_1 {
  float3 vPosition_param : TEXCOORD0;
  float2 vUV_param : TEXCOORD1;
  float2 tUV_param : TEXCOORD2;
  float2 stageUnits_1_param : TEXCOORD3;
  float2 levelUnits_param : TEXCOORD4;
  float2 tileID_1_param : TEXCOORD5;
};
struct tint_symbol_2 {
  float4 glFragColor_1 : SV_Target0;
};

main_out main_inner(float2 tUV_param, float2 tileID_1_param, float2 levelUnits_param, float2 stageUnits_1_param, float3 vPosition_param, float2 vUV_param) {
  tUV = tUV_param;
  tileID_1 = tileID_1_param;
  levelUnits = levelUnits_param;
  stageUnits_1 = stageUnits_1_param;
  vPosition = vPosition_param;
  vUV = vUV_param;
  main_1();
  main_out tint_symbol_3 = {glFragColor};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  main_out inner_result = main_inner(tint_symbol.tUV_param, tint_symbol.tileID_1_param, tint_symbol.levelUnits_param, tint_symbol.stageUnits_1_param, tint_symbol.vPosition_param, tint_symbol.vUV_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.glFragColor_1 = inner_result.glFragColor_1;
  return wrapper_result;
}
