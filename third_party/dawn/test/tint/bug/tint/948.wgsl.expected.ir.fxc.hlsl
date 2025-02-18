struct main_out {
  float4 glFragColor_1;
};

struct main_outputs {
  float4 main_out_glFragColor_1 : SV_Target0;
};

struct main_inputs {
  float3 vPosition_param : TEXCOORD0;
  float2 vUV_param : TEXCOORD1;
  float2 tUV_param : TEXCOORD2;
  float2 stageUnits_1_param : TEXCOORD3;
  float2 levelUnits_param : TEXCOORD4;
  float2 tileID_1_param : TEXCOORD5;
};


cbuffer cbuffer_x_20 : register(b9, space2) {
  uint4 x_20[8];
};
Texture2D<float4> frameMapTexture : register(t3, space2);
SamplerState frameMapSampler : register(s2, space2);
static float2 tUV = (0.0f).xx;
Texture2D<float4> tileMapsTexture0 : register(t5, space2);
SamplerState tileMapsSampler : register(s4, space2);
Texture2D<float4> tileMapsTexture1 : register(t6, space2);
Texture2D<float4> animationMapTexture : register(t8, space2);
SamplerState animationMapSampler : register(s7, space2);
static float mt = 0.0f;
Texture2D<float4> spriteSheetTexture : register(t1, space2);
SamplerState spriteSheetSampler : register(s0, space2);
static float4 glFragColor = (0.0f).xxxx;
static float2 tileID_1 = (0.0f).xx;
static float2 levelUnits = (0.0f).xx;
static float2 stageUnits_1 = (0.0f).xx;
static float3 vPosition = (0.0f).xxx;
static float2 vUV = (0.0f).xx;
float4x4 getFrameData_f1_(inout float frameID) {
  float fX = 0.0f;
  float x_15 = frameID;
  float x_25 = asfloat(x_20[6u].w);
  fX = (x_15 / x_25);
  float x_37 = fX;
  float4 x_40 = frameMapTexture.SampleBias(frameMapSampler, float2(x_37, 0.0f), clamp(0.0f, -16.0f, 15.9899997711181640625f));
  float x_44 = fX;
  float4 x_47 = frameMapTexture.SampleBias(frameMapSampler, float2(x_44, 0.25f), clamp(0.0f, -16.0f, 15.9899997711181640625f));
  float x_51 = fX;
  float4 x_54 = frameMapTexture.SampleBias(frameMapSampler, float2(x_51, 0.5f), clamp(0.0f, -16.0f, 15.9899997711181640625f));
  float4 v = float4(x_40.x, x_40.y, x_40.z, x_40.w);
  float4 v_1 = float4(x_47.x, x_47.y, x_47.z, x_47.w);
  return float4x4(v, v_1, float4(x_54.x, x_54.y, x_54.z, x_54.w), (0.0f).xxxx);
}

void main_1() {
  float4 color = (0.0f).xxxx;
  float2 tileUV = (0.0f).xx;
  float2 tileID = (0.0f).xx;
  float2 sheetUnits = (0.0f).xx;
  float spriteUnits = 0.0f;
  float2 stageUnits = (0.0f).xx;
  int i = int(0);
  float frameID_1 = 0.0f;
  float4 animationData = (0.0f).xxxx;
  float f = 0.0f;
  float4x4 frameData = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float param = 0.0f;
  float2 frameSize = (0.0f).xx;
  float2 offset_1 = (0.0f).xx;
  float2 ratio = (0.0f).xx;
  float4 nc = (0.0f).xxxx;
  float alpha = 0.0f;
  float3 mixed = (0.0f).xxx;
  color = (0.0f).xxxx;
  float2 x_86 = tUV;
  tileUV = frac(x_86);
  float x_91 = tileUV.y;
  tileUV.y = (1.0f - x_91);
  float2 x_95 = tUV;
  tileID = floor(x_95);
  float2 x_101 = asfloat(x_20[6u].xy);
  sheetUnits = ((1.0f).xx / x_101);
  float x_106 = asfloat(x_20[6u].w);
  spriteUnits = (1.0f / x_106);
  float2 x_111 = asfloat(x_20[5u].zw);
  stageUnits = ((1.0f).xx / x_111);
  i = int(0);
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      int x_122 = i;
      if ((x_122 < int(2))) {
      } else {
        break;
      }
      int x_126 = i;
      switch(x_126) {
        case int(1):
        {
          float2 x_150 = tileID;
          float2 x_154 = asfloat(x_20[5u].zw);
          float4 x_156 = tileMapsTexture1.SampleBias(tileMapsSampler, ((x_150 + (0.5f).xx) / x_154), clamp(0.0f, -16.0f, 15.9899997711181640625f));
          frameID_1 = x_156.x;
          break;
        }
        case int(0):
        {
          float2 x_136 = tileID;
          float2 x_140 = asfloat(x_20[5u].zw);
          float4 x_142 = tileMapsTexture0.SampleBias(tileMapsSampler, ((x_136 + (0.5f).xx) / x_140), clamp(0.0f, -16.0f, 15.9899997711181640625f));
          frameID_1 = x_142.x;
          break;
        }
        default:
        {
          break;
        }
      }
      float x_166 = frameID_1;
      float x_169 = asfloat(x_20[6u].w);
      float4 x_172 = animationMapTexture.SampleBias(animationMapSampler, float2(((x_166 + 0.5f) / x_169), 0.0f), clamp(0.0f, -16.0f, 15.9899997711181640625f));
      animationData = x_172;
      float x_174 = animationData.y;
      if ((x_174 > 0.0f)) {
        float x_181 = asfloat(x_20[0u].x);
        float x_184 = animationData.z;
        float v_2 = ((x_181 * x_184) / 1.0f);
        mt = ((x_181 * x_184) - ((((v_2 < 0.0f)) ? (ceil(v_2)) : (floor(v_2))) * 1.0f));
        f = 0.0f;
        {
          uint2 tint_loop_idx_1 = (0u).xx;
          while(true) {
            if (all((tint_loop_idx_1 == (4294967295u).xx))) {
              break;
            }
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
            float x_211 = asfloat(x_20[6u].w);
            float x_214 = f;
            float4 x_217 = (0.0f).xxxx;
            animationData = x_217;
            {
              uint tint_low_inc_1 = (tint_loop_idx_1.x + 1u);
              tint_loop_idx_1.x = tint_low_inc_1;
              uint tint_carry_1 = uint((tint_low_inc_1 == 0u));
              tint_loop_idx_1.y = (tint_loop_idx_1.y + tint_carry_1);
              float x_218 = f;
              f = (x_218 + 1.0f);
            }
            continue;
          }
        }
      }
      float x_222 = frameID_1;
      param = (x_222 + 0.5f);
      float4x4 x_225 = getFrameData_f1_(param);
      frameData = x_225;
      float4 x_228 = frameData[0u];
      float2 x_231 = asfloat(x_20[6u].xy);
      frameSize = (float2(x_228.w, x_228.z) / x_231);
      float4 x_235 = frameData[0u];
      float2 x_237 = sheetUnits;
      offset_1 = (float2(x_235.x, x_235.y) * x_237);
      float4 x_241 = frameData[2u];
      float4 x_244 = frameData[0u];
      float2 v_3 = float2(x_241.x, x_241.y);
      ratio = (v_3 / float2(x_244.w, x_244.z));
      float x_248 = frameData[2u].z;
      if ((x_248 == 1.0f)) {
        float2 x_252 = tileUV;
        tileUV = float2(x_252.y, x_252.x);
      }
      int x_254 = i;
      if ((x_254 == int(0))) {
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
        float3 v_4 = float3(x_290.x, x_290.y, x_290.z);
        float3 v_5 = float3(x_292.x, x_292.y, x_292.z);
        mixed = lerp(v_4, v_5, float3(x_295, x_295, x_295));
        float3 x_298 = mixed;
        float x_299 = alpha;
        color = float4(x_298.x, x_298.y, x_298.z, x_299);
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        int x_304 = i;
        i = (x_304 + int(1));
      }
      continue;
    }
  }
  float3 x_310 = asfloat(x_20[7u].xyz);
  float4 x_311 = color;
  float3 x_313 = (float3(x_311.x, x_311.y, x_311.z) * x_310);
  float4 x_314 = color;
  color = float4(x_313.x, x_313.y, x_313.z, x_314.w);
  float4 x_318 = color;
  glFragColor = x_318;
}

main_out main_inner(float2 tUV_param, float2 tileID_1_param, float2 levelUnits_param, float2 stageUnits_1_param, float3 vPosition_param, float2 vUV_param) {
  tUV = tUV_param;
  tileID_1 = tileID_1_param;
  levelUnits = levelUnits_param;
  stageUnits_1 = stageUnits_1_param;
  vPosition = vPosition_param;
  vUV = vUV_param;
  main_1();
  main_out v_6 = {glFragColor};
  return v_6;
}

main_outputs main(main_inputs inputs) {
  main_out v_7 = main_inner(inputs.tUV_param, inputs.tileID_1_param, inputs.levelUnits_param, inputs.stageUnits_1_param, inputs.vPosition_param, inputs.vUV_param);
  main_outputs v_8 = {v_7.glFragColor_1};
  return v_8;
}

