struct lightingInfo {
  float3 diffuse;
  float3 specular;
};

struct main_out {
  float4 glFragColor_1;
};

struct main_outputs {
  float4 main_out_glFragColor_1 : SV_Target0;
};

struct main_inputs {
  float4 v_output1_param : TEXCOORD0;
  float2 vMainuv_param : TEXCOORD1;
  float4 v_output2_param : TEXCOORD2;
  float2 v_uv_param : TEXCOORD3;
  bool gl_FrontFacing_param : SV_IsFrontFace;
};


static float u_Float = 0.0f;
static float3 u_Color = (0.0f).xxx;
Texture2D<float4> TextureSamplerTexture : register(t1, space2);
SamplerState TextureSamplerSampler : register(s0, space2);
static float2 vMainuv = (0.0f).xx;
cbuffer cbuffer_x_269 : register(b6, space2) {
  uint4 x_269[11];
};
static float4 v_output1 = (0.0f).xxxx;
static bool gl_FrontFacing = false;
static float2 v_uv = (0.0f).xx;
static float4 v_output2 = (0.0f).xxxx;
Texture2D<float4> TextureSampler1Texture : register(t3, space2);
SamplerState TextureSampler1Sampler : register(s2, space2);
cbuffer cbuffer_light0 : register(b5) {
  uint4 light0[6];
};
static float4 glFragColor = (0.0f).xxxx;
float3x3 cotangent_frame_vf3_vf3_vf2_vf2_(inout float3 normal_1, inout float3 p, inout float2 uv, inout float2 tangentSpaceParams) {
  float3 dp1 = (0.0f).xxx;
  float3 dp2 = (0.0f).xxx;
  float2 duv1 = (0.0f).xx;
  float2 duv2 = (0.0f).xx;
  float3 dp2perp = (0.0f).xxx;
  float3 dp1perp = (0.0f).xxx;
  float3 tangent = (0.0f).xxx;
  float3 bitangent = (0.0f).xxx;
  float invmax = 0.0f;
  float3 x_133 = p;
  dp1 = ddx(x_133);
  float3 x_136 = p;
  dp2 = ddy(x_136);
  float2 x_139 = uv;
  duv1 = ddx(x_139);
  float2 x_142 = uv;
  duv2 = ddy(x_142);
  float3 x_145 = dp2;
  float3 x_146 = normal_1;
  dp2perp = cross(x_145, x_146);
  float3 x_149 = normal_1;
  float3 x_150 = dp1;
  dp1perp = cross(x_149, x_150);
  float3 x_153 = dp2perp;
  float x_155 = duv1.x;
  float3 x_157 = dp1perp;
  float x_159 = duv2.x;
  tangent = ((x_153 * x_155) + (x_157 * x_159));
  float3 x_163 = dp2perp;
  float x_165 = duv1.y;
  float3 x_167 = dp1perp;
  float x_169 = duv2.y;
  bitangent = ((x_163 * x_165) + (x_167 * x_169));
  float x_173 = tangentSpaceParams.x;
  float3 x_174 = tangent;
  tangent = (x_174 * x_173);
  float x_177 = tangentSpaceParams.y;
  float3 x_178 = bitangent;
  bitangent = (x_178 * x_177);
  float3 x_181 = tangent;
  float3 x_182 = tangent;
  float3 x_184 = bitangent;
  float3 x_185 = bitangent;
  invmax = rsqrt(max(dot(x_181, x_182), dot(x_184, x_185)));
  float3 x_189 = tangent;
  float x_190 = invmax;
  float3 x_191 = (x_189 * x_190);
  float3 x_192 = bitangent;
  float x_193 = invmax;
  float3 x_194 = (x_192 * x_193);
  float3 x_195 = normal_1;
  float3 v = float3(x_191.x, x_191.y, x_191.z);
  float3 v_1 = float3(x_194.x, x_194.y, x_194.z);
  return float3x3(v, v_1, float3(x_195.x, x_195.y, x_195.z));
}

float3x3 transposeMat3_mf33_(inout float3x3 inMatrix) {
  float3 i0 = (0.0f).xxx;
  float3 i1 = (0.0f).xxx;
  float3 i2 = (0.0f).xxx;
  float3x3 outMatrix = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float3 x_60 = inMatrix[0u];
  i0 = x_60;
  float3 x_64 = inMatrix[1u];
  i1 = x_64;
  float3 x_68 = inMatrix[2u];
  i2 = x_68;
  float x_73 = i0.x;
  float x_75 = i1.x;
  float x_77 = i2.x;
  float3 x_78 = float3(x_73, x_75, x_77);
  float x_81 = i0.y;
  float x_83 = i1.y;
  float x_85 = i2.y;
  float3 x_86 = float3(x_81, x_83, x_85);
  float x_89 = i0.z;
  float x_91 = i1.z;
  float x_93 = i2.z;
  float3 x_94 = float3(x_89, x_91, x_93);
  float3 v_2 = float3(x_78.x, x_78.y, x_78.z);
  float3 v_3 = float3(x_86.x, x_86.y, x_86.z);
  outMatrix = float3x3(v_2, v_3, float3(x_94.x, x_94.y, x_94.z));
  float3x3 x_110 = outMatrix;
  return x_110;
}

float3 perturbNormalBase_mf33_vf3_f1_(inout float3x3 cotangentFrame, inout float3 normal, inout float scale) {
  float3x3 x_113 = cotangentFrame;
  float3 x_114 = normal;
  return normalize(mul(x_114, x_113));
}

float3 perturbNormal_mf33_vf3_f1_(inout float3x3 cotangentFrame_1, inout float3 textureSample, inout float scale_1) {
  float3x3 param = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float3 param_1 = (0.0f).xxx;
  float param_2 = 0.0f;
  float3 x_119 = textureSample;
  float3x3 x_125 = cotangentFrame_1;
  param = x_125;
  param_1 = ((x_119 * 2.0f) - (1.0f).xxx);
  float x_128 = scale_1;
  param_2 = x_128;
  float3 x_129 = perturbNormalBase_mf33_vf3_f1_(param, param_1, param_2);
  return x_129;
}

lightingInfo computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(inout float3 viewDirectionW, inout float3 vNormal, inout float4 lightData, inout float3 diffuseColor, inout float3 specularColor, inout float3 groundColor, inout float glossiness) {
  float ndl = 0.0f;
  lightingInfo result = (lightingInfo)0;
  float3 angleW = (0.0f).xxx;
  float specComp = 0.0f;
  float3 x_212 = vNormal;
  float4 x_213 = lightData;
  ndl = ((dot(x_212, float3(x_213.x, x_213.y, x_213.z)) * 0.5f) + 0.5f);
  float3 x_220 = groundColor;
  float3 x_221 = diffuseColor;
  float x_222 = ndl;
  result.diffuse = lerp(x_220, x_221, float3(x_222, x_222, x_222));
  float3 x_227 = viewDirectionW;
  float4 x_228 = lightData;
  angleW = normalize((x_227 + float3(x_228.x, x_228.y, x_228.z)));
  float3 x_233 = vNormal;
  float3 x_234 = angleW;
  specComp = max(0.0f, dot(x_233, x_234));
  float x_237 = specComp;
  float x_238 = glossiness;
  specComp = pow(x_237, max(1.0f, x_238));
  float x_241 = specComp;
  float3 x_242 = specularColor;
  result.specular = (x_242 * x_241);
  lightingInfo x_245 = result;
  return x_245;
}

void main_1() {
  float4 tempTextureRead = (0.0f).xxxx;
  float3 rgb = (0.0f).xxx;
  float3 output5 = (0.0f).xxx;
  float4 output4 = (0.0f).xxxx;
  float2 uvOffset = (0.0f).xx;
  float normalScale = 0.0f;
  float2 TBNUV = (0.0f).xx;
  float2 x_299 = (0.0f).xx;
  float3x3 TBN = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float3 param_3 = (0.0f).xxx;
  float3 param_4 = (0.0f).xxx;
  float2 param_5 = (0.0f).xx;
  float2 param_6 = (0.0f).xx;
  float3x3 invTBN = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float3x3 param_7 = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float parallaxLimit = 0.0f;
  float2 vOffsetDir = (0.0f).xx;
  float2 vMaxOffset = (0.0f).xx;
  float numSamples = 0.0f;
  float stepSize = 0.0f;
  float currRayHeight = 0.0f;
  float2 vCurrOffset = (0.0f).xx;
  float2 vLastOffset = (0.0f).xx;
  float lastSampledHeight = 0.0f;
  float currSampledHeight = 0.0f;
  int i = int(0);
  float delta1 = 0.0f;
  float delta2 = 0.0f;
  float ratio = 0.0f;
  float2 parallaxOcclusion_0 = (0.0f).xx;
  float3x3 param_8 = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float3 param_9 = (0.0f).xxx;
  float param_10 = 0.0f;
  float2 output6 = (0.0f).xx;
  float4 tempTextureRead1 = (0.0f).xxxx;
  float3 rgb1 = (0.0f).xxx;
  float3 viewDirectionW_1 = (0.0f).xxx;
  float shadow = 0.0f;
  float glossiness_1 = 0.0f;
  float3 diffuseBase = (0.0f).xxx;
  float3 specularBase = (0.0f).xxx;
  float3 normalW = (0.0f).xxx;
  lightingInfo info = (lightingInfo)0;
  float3 param_11 = (0.0f).xxx;
  float3 param_12 = (0.0f).xxx;
  float4 param_13 = (0.0f).xxxx;
  float3 param_14 = (0.0f).xxx;
  float3 param_15 = (0.0f).xxx;
  float3 param_16 = (0.0f).xxx;
  float param_17 = 0.0f;
  float3 diffuseOutput = (0.0f).xxx;
  float3 specularOutput = (0.0f).xxx;
  float3 output3 = (0.0f).xxx;
  u_Float = 100.0f;
  u_Color = (0.5f).xxx;
  float2 x_261 = vMainuv;
  float4 x_262 = TextureSamplerTexture.Sample(TextureSamplerSampler, x_261);
  tempTextureRead = x_262;
  float4 x_264 = tempTextureRead;
  float x_273 = asfloat(x_269[10u].x);
  rgb = (float3(x_264.x, x_264.y, x_264.z) * x_273);
  float3 x_279 = asfloat(x_269[9u].xyz);
  float4 x_282 = v_output1;
  output5 = normalize((x_279 - float3(x_282.x, x_282.y, x_282.z)));
  output4 = (0.0f).xxxx;
  uvOffset = (0.0f).xx;
  float x_292 = asfloat(x_269[8u].x);
  normalScale = (1.0f / x_292);
  bool x_298 = gl_FrontFacing;
  if (x_298) {
    float2 x_303 = v_uv;
    x_299 = x_303;
  } else {
    float2 x_305 = v_uv;
    x_299 = -(x_305);
  }
  float2 x_307 = x_299;
  TBNUV = x_307;
  float4 x_310 = v_output2;
  float x_312 = normalScale;
  param_3 = (float3(x_310.x, x_310.y, x_310.z) * x_312);
  float4 x_317 = v_output1;
  param_4 = float3(x_317.x, x_317.y, x_317.z);
  float2 x_320 = TBNUV;
  param_5 = x_320;
  float2 x_324 = asfloat(x_269[10u].zw);
  param_6 = x_324;
  float3x3 x_325 = cotangent_frame_vf3_vf3_vf2_vf2_(param_3, param_4, param_5, param_6);
  TBN = x_325;
  float3x3 x_328 = TBN;
  param_7 = x_328;
  float3x3 x_329 = transposeMat3_mf33_(param_7);
  invTBN = x_329;
  float3x3 x_331 = invTBN;
  float3 x_332 = output5;
  float3 x_334 = mul(-(x_332), x_331);
  float3x3 x_337 = invTBN;
  float3 x_338 = output5;
  parallaxLimit = (length(float2(x_334.x, x_334.y)) / mul(-(x_338), x_337).z);
  float x_345 = asfloat(x_269[9u].w);
  float x_346 = parallaxLimit;
  parallaxLimit = (x_346 * x_345);
  float3x3 x_349 = invTBN;
  float3 x_350 = output5;
  float3 x_352 = mul(-(x_350), x_349);
  vOffsetDir = normalize(float2(x_352.x, x_352.y));
  float2 x_356 = vOffsetDir;
  float x_357 = parallaxLimit;
  vMaxOffset = (x_356 * x_357);
  float3x3 x_361 = invTBN;
  float3 x_362 = output5;
  float3x3 x_365 = invTBN;
  float4 x_366 = v_output2;
  numSamples = (15.0f + (dot(mul(-(x_362), x_361), mul(float3(x_366.x, x_366.y, x_366.z), x_365)) * -11.0f));
  float x_374 = numSamples;
  stepSize = (1.0f / x_374);
  currRayHeight = 1.0f;
  vCurrOffset = (0.0f).xx;
  vLastOffset = (0.0f).xx;
  lastSampledHeight = 1.0f;
  currSampledHeight = 1.0f;
  i = int(0);
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      int x_388 = i;
      if ((x_388 < int(15))) {
      } else {
        break;
      }
      float2 x_394 = v_uv;
      float2 x_395 = vCurrOffset;
      float4 x_397 = (0.0f).xxxx;
      currSampledHeight = x_397.w;
      float x_400 = currSampledHeight;
      float x_401 = currRayHeight;
      if ((x_400 > x_401)) {
        float x_406 = currSampledHeight;
        float x_407 = currRayHeight;
        delta1 = (x_406 - x_407);
        float x_410 = currRayHeight;
        float x_411 = stepSize;
        float x_413 = lastSampledHeight;
        delta2 = ((x_410 + x_411) - x_413);
        float x_416 = delta1;
        float x_417 = delta1;
        float x_418 = delta2;
        ratio = (x_416 / (x_417 + x_418));
        float x_421 = ratio;
        float2 x_422 = vLastOffset;
        float x_424 = ratio;
        float2 x_426 = vCurrOffset;
        vCurrOffset = ((x_422 * x_421) + (x_426 * (1.0f - x_424)));
        break;
      } else {
        float x_431 = stepSize;
        float x_432 = currRayHeight;
        currRayHeight = (x_432 - x_431);
        float2 x_434 = vCurrOffset;
        vLastOffset = x_434;
        float x_435 = stepSize;
        float2 x_436 = vMaxOffset;
        float2 x_438 = vCurrOffset;
        vCurrOffset = (x_438 + (x_436 * x_435));
        float x_440 = currSampledHeight;
        lastSampledHeight = x_440;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        int x_441 = i;
        i = (x_441 + int(1));
      }
      continue;
    }
  }
  float2 x_444 = vCurrOffset;
  parallaxOcclusion_0 = x_444;
  float2 x_445 = parallaxOcclusion_0;
  uvOffset = x_445;
  float2 x_449 = v_uv;
  float2 x_450 = uvOffset;
  float4 x_452 = TextureSamplerTexture.Sample(TextureSamplerSampler, (x_449 + x_450));
  float x_454 = asfloat(x_269[8u].x);
  float3x3 x_457 = TBN;
  param_8 = x_457;
  param_9 = float3(x_452.x, x_452.y, x_452.z);
  param_10 = (1.0f / x_454);
  float3 x_461 = perturbNormal_mf33_vf3_f1_(param_8, param_9, param_10);
  float4 x_462 = output4;
  output4 = float4(x_461.x, x_461.y, x_461.z, x_462.w);
  float2 x_465 = v_uv;
  float2 x_466 = uvOffset;
  output6 = (x_465 + x_466);
  float2 x_474 = output6;
  float4 x_475 = TextureSampler1Texture.Sample(TextureSampler1Sampler, x_474);
  tempTextureRead1 = x_475;
  float4 x_477 = tempTextureRead1;
  rgb1 = float3(x_477.x, x_477.y, x_477.z);
  float3 x_481 = asfloat(x_269[9u].xyz);
  float4 x_482 = v_output1;
  viewDirectionW_1 = normalize((x_481 - float3(x_482.x, x_482.y, x_482.z)));
  shadow = 1.0f;
  float x_488 = u_Float;
  glossiness_1 = (1.0f * x_488);
  diffuseBase = (0.0f).xxx;
  specularBase = (0.0f).xxx;
  float4 x_494 = output4;
  normalW = float3(x_494.x, x_494.y, x_494.z);
  float3 x_501 = viewDirectionW_1;
  param_11 = x_501;
  float3 x_503 = normalW;
  param_12 = x_503;
  float4 x_507 = asfloat(light0[0u]);
  param_13 = x_507;
  float4 x_510 = asfloat(light0[1u]);
  param_14 = float3(x_510.x, x_510.y, x_510.z);
  float4 x_514 = asfloat(light0[2u]);
  param_15 = float3(x_514.x, x_514.y, x_514.z);
  float3 x_518 = asfloat(light0[3u].xyz);
  param_16 = x_518;
  float x_520 = glossiness_1;
  param_17 = x_520;
  lightingInfo x_521 = computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(param_11, param_12, param_13, param_14, param_15, param_16, param_17);
  info = x_521;
  shadow = 1.0f;
  float3 x_523 = info.diffuse;
  float x_524 = shadow;
  float3 x_526 = diffuseBase;
  diffuseBase = (x_526 + (x_523 * x_524));
  float3 x_529 = info.specular;
  float x_530 = shadow;
  float3 x_532 = specularBase;
  specularBase = (x_532 + (x_529 * x_530));
  float3 x_535 = diffuseBase;
  float3 x_536 = rgb1;
  diffuseOutput = (x_535 * x_536);
  float3 x_539 = specularBase;
  float3 x_540 = u_Color;
  specularOutput = (x_539 * x_540);
  float3 x_543 = diffuseOutput;
  float3 x_544 = specularOutput;
  output3 = (x_543 + x_544);
  float3 x_548 = output3;
  glFragColor = float4(x_548.x, x_548.y, x_548.z, 1.0f);
}

main_out main_inner(float2 vMainuv_param, float4 v_output1_param, bool gl_FrontFacing_param, float2 v_uv_param, float4 v_output2_param) {
  vMainuv = vMainuv_param;
  v_output1 = v_output1_param;
  gl_FrontFacing = gl_FrontFacing_param;
  v_uv = v_uv_param;
  v_output2 = v_output2_param;
  main_1();
  main_out v_4 = {glFragColor};
  return v_4;
}

main_outputs main(main_inputs inputs) {
  main_out v_5 = main_inner(inputs.vMainuv_param, inputs.v_output1_param, inputs.gl_FrontFacing_param, inputs.v_uv_param, inputs.v_output2_param);
  main_outputs v_6 = {v_5.glFragColor_1};
  return v_6;
}

