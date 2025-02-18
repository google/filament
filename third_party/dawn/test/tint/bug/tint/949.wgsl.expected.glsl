#version 310 es
precision highp float;
precision highp int;


struct LeftOver {
  mat4 u_World;
  mat4 u_ViewProjection;
  float u_bumpStrength;
  uint padding;
  uint tint_pad_0;
  uint tint_pad_1;
  vec3 u_cameraPosition;
  float u_parallaxScale;
  float textureInfoName;
  uint padding_1;
  vec2 tangentSpaceParameter0;
};

struct Light0 {
  vec4 vLightData;
  vec4 vLightDiffuse;
  vec4 vLightSpecular;
  vec3 vLightGround;
  uint padding_2;
  vec4 shadowsInfo;
  vec2 depthValues;
  uint tint_pad_0;
  uint tint_pad_1;
};

struct lightingInfo {
  vec3 diffuse;
  vec3 specular;
};

struct main_out {
  vec4 glFragColor_1;
};

float u_Float = 0.0f;
vec3 u_Color = vec3(0.0f);
vec2 vMainuv = vec2(0.0f);
layout(binding = 6, std140)
uniform f_x_269_block_ubo {
  LeftOver inner;
} v;
vec4 v_output1 = vec4(0.0f);
bool v_1 = false;
vec2 v_uv = vec2(0.0f);
vec4 v_output2 = vec4(0.0f);
layout(binding = 5, std140)
uniform f_light0_block_ubo {
  Light0 inner;
} v_2;
vec4 glFragColor = vec4(0.0f);
uniform highp sampler2D TextureSamplerTexture_TextureSamplerSampler;
uniform highp sampler2D TextureSampler1Texture_TextureSampler1Sampler;
layout(location = 1) in vec2 tint_interstage_location1;
layout(location = 0) in vec4 tint_interstage_location0;
layout(location = 3) in vec2 tint_interstage_location3;
layout(location = 2) in vec4 tint_interstage_location2;
layout(location = 0) out vec4 main_loc0_Output;
mat3 cotangent_frame_vf3_vf3_vf2_vf2_(inout vec3 normal_1, inout vec3 p, inout vec2 uv, inout vec2 tangentSpaceParams) {
  vec3 dp1 = vec3(0.0f);
  vec3 dp2 = vec3(0.0f);
  vec2 duv1 = vec2(0.0f);
  vec2 duv2 = vec2(0.0f);
  vec3 dp2perp = vec3(0.0f);
  vec3 dp1perp = vec3(0.0f);
  vec3 tangent = vec3(0.0f);
  vec3 bitangent = vec3(0.0f);
  float invmax = 0.0f;
  vec3 x_133 = p;
  dp1 = dFdx(x_133);
  vec3 x_136 = p;
  dp2 = dFdy(x_136);
  vec2 x_139 = uv;
  duv1 = dFdx(x_139);
  vec2 x_142 = uv;
  duv2 = dFdy(x_142);
  vec3 x_145 = dp2;
  vec3 x_146 = normal_1;
  dp2perp = cross(x_145, x_146);
  vec3 x_149 = normal_1;
  vec3 x_150 = dp1;
  dp1perp = cross(x_149, x_150);
  vec3 x_153 = dp2perp;
  float x_155 = duv1.x;
  vec3 x_157 = dp1perp;
  float x_159 = duv2.x;
  tangent = ((x_153 * x_155) + (x_157 * x_159));
  vec3 x_163 = dp2perp;
  float x_165 = duv1.y;
  vec3 x_167 = dp1perp;
  float x_169 = duv2.y;
  bitangent = ((x_163 * x_165) + (x_167 * x_169));
  float x_173 = tangentSpaceParams.x;
  vec3 x_174 = tangent;
  tangent = (x_174 * x_173);
  float x_177 = tangentSpaceParams.y;
  vec3 x_178 = bitangent;
  bitangent = (x_178 * x_177);
  vec3 x_181 = tangent;
  vec3 x_182 = tangent;
  vec3 x_184 = bitangent;
  vec3 x_185 = bitangent;
  invmax = inversesqrt(max(dot(x_181, x_182), dot(x_184, x_185)));
  vec3 x_189 = tangent;
  float x_190 = invmax;
  vec3 x_191 = (x_189 * x_190);
  vec3 x_192 = bitangent;
  float x_193 = invmax;
  vec3 x_194 = (x_192 * x_193);
  vec3 x_195 = normal_1;
  vec3 v_3 = vec3(x_191.x, x_191.y, x_191.z);
  vec3 v_4 = vec3(x_194.x, x_194.y, x_194.z);
  return mat3(v_3, v_4, vec3(x_195.x, x_195.y, x_195.z));
}
mat3 transposeMat3_mf33_(inout mat3 inMatrix) {
  vec3 i0 = vec3(0.0f);
  vec3 i1 = vec3(0.0f);
  vec3 i2 = vec3(0.0f);
  mat3 outMatrix = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 x_60 = inMatrix[0u];
  i0 = x_60;
  vec3 x_64 = inMatrix[1u];
  i1 = x_64;
  vec3 x_68 = inMatrix[2u];
  i2 = x_68;
  float x_73 = i0.x;
  float x_75 = i1.x;
  float x_77 = i2.x;
  vec3 x_78 = vec3(x_73, x_75, x_77);
  float x_81 = i0.y;
  float x_83 = i1.y;
  float x_85 = i2.y;
  vec3 x_86 = vec3(x_81, x_83, x_85);
  float x_89 = i0.z;
  float x_91 = i1.z;
  float x_93 = i2.z;
  vec3 x_94 = vec3(x_89, x_91, x_93);
  vec3 v_5 = vec3(x_78.x, x_78.y, x_78.z);
  vec3 v_6 = vec3(x_86.x, x_86.y, x_86.z);
  outMatrix = mat3(v_5, v_6, vec3(x_94.x, x_94.y, x_94.z));
  mat3 x_110 = outMatrix;
  return x_110;
}
vec3 perturbNormalBase_mf33_vf3_f1_(inout mat3 cotangentFrame, inout vec3 normal, inout float scale) {
  mat3 x_113 = cotangentFrame;
  vec3 x_114 = normal;
  return normalize((x_113 * x_114));
}
vec3 perturbNormal_mf33_vf3_f1_(inout mat3 cotangentFrame_1, inout vec3 textureSample, inout float scale_1) {
  mat3 param = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 param_1 = vec3(0.0f);
  float param_2 = 0.0f;
  vec3 x_119 = textureSample;
  mat3 x_125 = cotangentFrame_1;
  param = x_125;
  param_1 = ((x_119 * 2.0f) - vec3(1.0f));
  float x_128 = scale_1;
  param_2 = x_128;
  vec3 x_129 = perturbNormalBase_mf33_vf3_f1_(param, param_1, param_2);
  return x_129;
}
lightingInfo computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(inout vec3 viewDirectionW, inout vec3 vNormal, inout vec4 lightData, inout vec3 diffuseColor, inout vec3 specularColor, inout vec3 groundColor, inout float glossiness) {
  float ndl = 0.0f;
  lightingInfo result = lightingInfo(vec3(0.0f), vec3(0.0f));
  vec3 angleW = vec3(0.0f);
  float specComp = 0.0f;
  vec3 x_212 = vNormal;
  vec4 x_213 = lightData;
  ndl = ((dot(x_212, vec3(x_213.x, x_213.y, x_213.z)) * 0.5f) + 0.5f);
  vec3 x_220 = groundColor;
  vec3 x_221 = diffuseColor;
  float x_222 = ndl;
  result.diffuse = mix(x_220, x_221, vec3(x_222, x_222, x_222));
  vec3 x_227 = viewDirectionW;
  vec4 x_228 = lightData;
  angleW = normalize((x_227 + vec3(x_228.x, x_228.y, x_228.z)));
  vec3 x_233 = vNormal;
  vec3 x_234 = angleW;
  specComp = max(0.0f, dot(x_233, x_234));
  float x_237 = specComp;
  float x_238 = glossiness;
  specComp = pow(x_237, max(1.0f, x_238));
  float x_241 = specComp;
  vec3 x_242 = specularColor;
  result.specular = (x_242 * x_241);
  lightingInfo x_245 = result;
  return x_245;
}
void main_1() {
  vec4 tempTextureRead = vec4(0.0f);
  vec3 rgb = vec3(0.0f);
  vec3 output5 = vec3(0.0f);
  vec4 output4 = vec4(0.0f);
  vec2 uvOffset = vec2(0.0f);
  float normalScale = 0.0f;
  vec2 TBNUV = vec2(0.0f);
  vec2 x_299 = vec2(0.0f);
  mat3 TBN = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 param_3 = vec3(0.0f);
  vec3 param_4 = vec3(0.0f);
  vec2 param_5 = vec2(0.0f);
  vec2 param_6 = vec2(0.0f);
  mat3 invTBN = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  mat3 param_7 = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  float parallaxLimit = 0.0f;
  vec2 vOffsetDir = vec2(0.0f);
  vec2 vMaxOffset = vec2(0.0f);
  float numSamples = 0.0f;
  float stepSize = 0.0f;
  float currRayHeight = 0.0f;
  vec2 vCurrOffset = vec2(0.0f);
  vec2 vLastOffset = vec2(0.0f);
  float lastSampledHeight = 0.0f;
  float currSampledHeight = 0.0f;
  int i = 0;
  float delta1 = 0.0f;
  float delta2 = 0.0f;
  float ratio = 0.0f;
  vec2 parallaxOcclusion_0 = vec2(0.0f);
  mat3 param_8 = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 param_9 = vec3(0.0f);
  float param_10 = 0.0f;
  vec2 output6 = vec2(0.0f);
  vec4 tempTextureRead1 = vec4(0.0f);
  vec3 rgb1 = vec3(0.0f);
  vec3 viewDirectionW_1 = vec3(0.0f);
  float shadow = 0.0f;
  float glossiness_1 = 0.0f;
  vec3 diffuseBase = vec3(0.0f);
  vec3 specularBase = vec3(0.0f);
  vec3 normalW = vec3(0.0f);
  lightingInfo info = lightingInfo(vec3(0.0f), vec3(0.0f));
  vec3 param_11 = vec3(0.0f);
  vec3 param_12 = vec3(0.0f);
  vec4 param_13 = vec4(0.0f);
  vec3 param_14 = vec3(0.0f);
  vec3 param_15 = vec3(0.0f);
  vec3 param_16 = vec3(0.0f);
  float param_17 = 0.0f;
  vec3 diffuseOutput = vec3(0.0f);
  vec3 specularOutput = vec3(0.0f);
  vec3 output3 = vec3(0.0f);
  u_Float = 100.0f;
  u_Color = vec3(0.5f);
  vec2 x_261 = vMainuv;
  vec4 x_262 = texture(TextureSamplerTexture_TextureSamplerSampler, x_261);
  tempTextureRead = x_262;
  vec4 x_264 = tempTextureRead;
  float x_273 = v.inner.textureInfoName;
  rgb = (vec3(x_264.x, x_264.y, x_264.z) * x_273);
  vec3 x_279 = v.inner.u_cameraPosition;
  vec4 x_282 = v_output1;
  output5 = normalize((x_279 - vec3(x_282.x, x_282.y, x_282.z)));
  output4 = vec4(0.0f);
  uvOffset = vec2(0.0f);
  float x_292 = v.inner.u_bumpStrength;
  normalScale = (1.0f / x_292);
  bool x_298 = v_1;
  if (x_298) {
    vec2 x_303 = v_uv;
    x_299 = x_303;
  } else {
    vec2 x_305 = v_uv;
    x_299 = -(x_305);
  }
  vec2 x_307 = x_299;
  TBNUV = x_307;
  vec4 x_310 = v_output2;
  float x_312 = normalScale;
  param_3 = (vec3(x_310.x, x_310.y, x_310.z) * x_312);
  vec4 x_317 = v_output1;
  param_4 = vec3(x_317.x, x_317.y, x_317.z);
  vec2 x_320 = TBNUV;
  param_5 = x_320;
  vec2 x_324 = v.inner.tangentSpaceParameter0;
  param_6 = x_324;
  mat3 x_325 = cotangent_frame_vf3_vf3_vf2_vf2_(param_3, param_4, param_5, param_6);
  TBN = x_325;
  mat3 x_328 = TBN;
  param_7 = x_328;
  mat3 x_329 = transposeMat3_mf33_(param_7);
  invTBN = x_329;
  mat3 x_331 = invTBN;
  vec3 x_332 = output5;
  vec3 x_334 = (x_331 * -(x_332));
  mat3 x_337 = invTBN;
  vec3 x_338 = output5;
  parallaxLimit = (length(vec2(x_334.x, x_334.y)) / (x_337 * -(x_338)).z);
  float x_345 = v.inner.u_parallaxScale;
  float x_346 = parallaxLimit;
  parallaxLimit = (x_346 * x_345);
  mat3 x_349 = invTBN;
  vec3 x_350 = output5;
  vec3 x_352 = (x_349 * -(x_350));
  vOffsetDir = normalize(vec2(x_352.x, x_352.y));
  vec2 x_356 = vOffsetDir;
  float x_357 = parallaxLimit;
  vMaxOffset = (x_356 * x_357);
  mat3 x_361 = invTBN;
  vec3 x_362 = output5;
  mat3 x_365 = invTBN;
  vec4 x_366 = v_output2;
  numSamples = (15.0f + (dot((x_361 * -(x_362)), (x_365 * vec3(x_366.x, x_366.y, x_366.z))) * -11.0f));
  float x_374 = numSamples;
  stepSize = (1.0f / x_374);
  currRayHeight = 1.0f;
  vCurrOffset = vec2(0.0f);
  vLastOffset = vec2(0.0f);
  lastSampledHeight = 1.0f;
  currSampledHeight = 1.0f;
  i = 0;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      int x_388 = i;
      if ((x_388 < 15)) {
      } else {
        break;
      }
      vec2 x_394 = v_uv;
      vec2 x_395 = vCurrOffset;
      vec4 x_397 = vec4(0.0f);
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
        vec2 x_422 = vLastOffset;
        float x_424 = ratio;
        vec2 x_426 = vCurrOffset;
        vCurrOffset = ((x_422 * x_421) + (x_426 * (1.0f - x_424)));
        break;
      } else {
        float x_431 = stepSize;
        float x_432 = currRayHeight;
        currRayHeight = (x_432 - x_431);
        vec2 x_434 = vCurrOffset;
        vLastOffset = x_434;
        float x_435 = stepSize;
        vec2 x_436 = vMaxOffset;
        vec2 x_438 = vCurrOffset;
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
        i = (x_441 + 1);
      }
      continue;
    }
  }
  vec2 x_444 = vCurrOffset;
  parallaxOcclusion_0 = x_444;
  vec2 x_445 = parallaxOcclusion_0;
  uvOffset = x_445;
  vec2 x_449 = v_uv;
  vec2 x_450 = uvOffset;
  vec4 x_452 = texture(TextureSamplerTexture_TextureSamplerSampler, (x_449 + x_450));
  float x_454 = v.inner.u_bumpStrength;
  mat3 x_457 = TBN;
  param_8 = x_457;
  param_9 = vec3(x_452.x, x_452.y, x_452.z);
  param_10 = (1.0f / x_454);
  vec3 x_461 = perturbNormal_mf33_vf3_f1_(param_8, param_9, param_10);
  vec4 x_462 = output4;
  output4 = vec4(x_461.x, x_461.y, x_461.z, x_462.w);
  vec2 x_465 = v_uv;
  vec2 x_466 = uvOffset;
  output6 = (x_465 + x_466);
  vec2 x_474 = output6;
  vec4 x_475 = texture(TextureSampler1Texture_TextureSampler1Sampler, x_474);
  tempTextureRead1 = x_475;
  vec4 x_477 = tempTextureRead1;
  rgb1 = vec3(x_477.x, x_477.y, x_477.z);
  vec3 x_481 = v.inner.u_cameraPosition;
  vec4 x_482 = v_output1;
  viewDirectionW_1 = normalize((x_481 - vec3(x_482.x, x_482.y, x_482.z)));
  shadow = 1.0f;
  float x_488 = u_Float;
  glossiness_1 = (1.0f * x_488);
  diffuseBase = vec3(0.0f);
  specularBase = vec3(0.0f);
  vec4 x_494 = output4;
  normalW = vec3(x_494.x, x_494.y, x_494.z);
  vec3 x_501 = viewDirectionW_1;
  param_11 = x_501;
  vec3 x_503 = normalW;
  param_12 = x_503;
  vec4 x_507 = v_2.inner.vLightData;
  param_13 = x_507;
  vec4 x_510 = v_2.inner.vLightDiffuse;
  param_14 = vec3(x_510.x, x_510.y, x_510.z);
  vec4 x_514 = v_2.inner.vLightSpecular;
  param_15 = vec3(x_514.x, x_514.y, x_514.z);
  vec3 x_518 = v_2.inner.vLightGround;
  param_16 = x_518;
  float x_520 = glossiness_1;
  param_17 = x_520;
  lightingInfo x_521 = computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(param_11, param_12, param_13, param_14, param_15, param_16, param_17);
  info = x_521;
  shadow = 1.0f;
  vec3 x_523 = info.diffuse;
  float x_524 = shadow;
  vec3 x_526 = diffuseBase;
  diffuseBase = (x_526 + (x_523 * x_524));
  vec3 x_529 = info.specular;
  float x_530 = shadow;
  vec3 x_532 = specularBase;
  specularBase = (x_532 + (x_529 * x_530));
  vec3 x_535 = diffuseBase;
  vec3 x_536 = rgb1;
  diffuseOutput = (x_535 * x_536);
  vec3 x_539 = specularBase;
  vec3 x_540 = u_Color;
  specularOutput = (x_539 * x_540);
  vec3 x_543 = diffuseOutput;
  vec3 x_544 = specularOutput;
  output3 = (x_543 + x_544);
  vec3 x_548 = output3;
  glFragColor = vec4(x_548.x, x_548.y, x_548.z, 1.0f);
}
main_out main_inner(vec2 vMainuv_param, vec4 v_output1_param, bool v_7, vec2 v_uv_param, vec4 v_output2_param) {
  vMainuv = vMainuv_param;
  v_output1 = v_output1_param;
  v_1 = v_7;
  v_uv = v_uv_param;
  v_output2 = v_output2_param;
  main_1();
  return main_out(glFragColor);
}
void main() {
  main_loc0_Output = main_inner(tint_interstage_location1, tint_interstage_location0, gl_FrontFacing, tint_interstage_location3, tint_interstage_location2).glFragColor_1;
}
