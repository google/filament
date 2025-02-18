// Dumped generated WGSL
struct lightingInfo {
  diffuse : vec3<f32>,
  specular : vec3<f32>,
};

struct LeftOver {
  u_World : mat4x4<f32>,
  u_ViewProjection : mat4x4<f32>,
  u_bumpStrength : f32,
  @size(12)
  padding : u32,
  u_cameraPosition : vec3<f32>,
  u_parallaxScale : f32,
  textureInfoName : f32,
  @size(4)
  padding_1 : u32,
  tangentSpaceParameter0 : vec2<f32>,
};

struct Light0 {
  vLightData : vec4<f32>,
  vLightDiffuse : vec4<f32>,
  vLightSpecular : vec4<f32>,
  vLightGround : vec3<f32>,
  @size(4)
  padding_2 : u32,
  shadowsInfo : vec4<f32>,
  depthValues : vec2<f32>,
};

var<private> u_Float : f32;

var<private> u_Color : vec3<f32>;

@group(2) @binding(1) var TextureSamplerTexture : texture_2d<f32>;

@group(2) @binding(0) var TextureSamplerSampler : sampler;

var<private> vMainuv : vec2<f32>;

@group(2) @binding(6) var<uniform> x_269 : LeftOver;

var<private> v_output1 : vec4<f32>;

var<private> gl_FrontFacing : bool;

var<private> v_uv : vec2<f32>;

var<private> v_output2 : vec4<f32>;

@group(2) @binding(3) var TextureSampler1Texture : texture_2d<f32>;

@group(2) @binding(2) var TextureSampler1Sampler : sampler;

@group(0) @binding(5) var<uniform> light0 : Light0;

var<private> glFragColor : vec4<f32>;

@group(2) @binding(4) var bumpSamplerSampler : sampler;

@group(2) @binding(5) var bumpSamplerTexture : texture_2d<f32>;

fn cotangent_frame_vf3_vf3_vf2_vf2_(normal_1 : ptr<function, vec3<f32>>, p : ptr<function, vec3<f32>>, uv : ptr<function, vec2<f32>>, tangentSpaceParams : ptr<function, vec2<f32>>) -> mat3x3<f32> {
  var dp1 : vec3<f32>;
  var dp2 : vec3<f32>;
  var duv1 : vec2<f32>;
  var duv2 : vec2<f32>;
  var dp2perp : vec3<f32>;
  var dp1perp : vec3<f32>;
  var tangent : vec3<f32>;
  var bitangent : vec3<f32>;
  var invmax : f32;
  let x_133 : vec3<f32> = *(p);
  dp1 = dpdx(x_133);
  let x_136 : vec3<f32> = *(p);
  dp2 = dpdy(x_136);
  let x_139 : vec2<f32> = *(uv);
  duv1 = dpdx(x_139);
  let x_142 : vec2<f32> = *(uv);
  duv2 = dpdy(x_142);
  let x_145 : vec3<f32> = dp2;
  let x_146 : vec3<f32> = *(normal_1);
  dp2perp = cross(x_145, x_146);
  let x_149 : vec3<f32> = *(normal_1);
  let x_150 : vec3<f32> = dp1;
  dp1perp = cross(x_149, x_150);
  let x_153 : vec3<f32> = dp2perp;
  let x_155 : f32 = duv1.x;
  let x_157 : vec3<f32> = dp1perp;
  let x_159 : f32 = duv2.x;
  tangent = ((x_153 * x_155) + (x_157 * x_159));
  let x_163 : vec3<f32> = dp2perp;
  let x_165 : f32 = duv1.y;
  let x_167 : vec3<f32> = dp1perp;
  let x_169 : f32 = duv2.y;
  bitangent = ((x_163 * x_165) + (x_167 * x_169));
  let x_173 : f32 = (*(tangentSpaceParams)).x;
  let x_174 : vec3<f32> = tangent;
  tangent = (x_174 * x_173);
  let x_177 : f32 = (*(tangentSpaceParams)).y;
  let x_178 : vec3<f32> = bitangent;
  bitangent = (x_178 * x_177);
  let x_181 : vec3<f32> = tangent;
  let x_182 : vec3<f32> = tangent;
  let x_184 : vec3<f32> = bitangent;
  let x_185 : vec3<f32> = bitangent;
  invmax = inverseSqrt(max(dot(x_181, x_182), dot(x_184, x_185)));
  let x_189 : vec3<f32> = tangent;
  let x_190 : f32 = invmax;
  let x_191 : vec3<f32> = (x_189 * x_190);
  let x_192 : vec3<f32> = bitangent;
  let x_193 : f32 = invmax;
  let x_194 : vec3<f32> = (x_192 * x_193);
  let x_195 : vec3<f32> = *(normal_1);
  return mat3x3<f32>(vec3<f32>(x_191.x, x_191.y, x_191.z), vec3<f32>(x_194.x, x_194.y, x_194.z), vec3<f32>(x_195.x, x_195.y, x_195.z));
}

fn transposeMat3_mf33_(inMatrix : ptr<function, mat3x3<f32>>) -> mat3x3<f32> {
  var i0 : vec3<f32>;
  var i1 : vec3<f32>;
  var i2 : vec3<f32>;
  var outMatrix : mat3x3<f32>;
  let x_60 : vec3<f32> = (*(inMatrix))[0];
  i0 = x_60;
  let x_64 : vec3<f32> = (*(inMatrix))[1];
  i1 = x_64;
  let x_68 : vec3<f32> = (*(inMatrix))[2];
  i2 = x_68;
  let x_73 : f32 = i0.x;
  let x_75 : f32 = i1.x;
  let x_77 : f32 = i2.x;
  let x_78 : vec3<f32> = vec3<f32>(x_73, x_75, x_77);
  let x_81 : f32 = i0.y;
  let x_83 : f32 = i1.y;
  let x_85 : f32 = i2.y;
  let x_86 : vec3<f32> = vec3<f32>(x_81, x_83, x_85);
  let x_89 : f32 = i0.z;
  let x_91 : f32 = i1.z;
  let x_93 : f32 = i2.z;
  let x_94 : vec3<f32> = vec3<f32>(x_89, x_91, x_93);
  outMatrix = mat3x3<f32>(vec3<f32>(x_78.x, x_78.y, x_78.z), vec3<f32>(x_86.x, x_86.y, x_86.z), vec3<f32>(x_94.x, x_94.y, x_94.z));
  let x_110 : mat3x3<f32> = outMatrix;
  return x_110;
}

fn perturbNormalBase_mf33_vf3_f1_(cotangentFrame : ptr<function, mat3x3<f32>>, normal : ptr<function, vec3<f32>>, scale : ptr<function, f32>) -> vec3<f32> {
  let x_113 : mat3x3<f32> = *(cotangentFrame);
  let x_114 : vec3<f32> = *(normal);
  return normalize((x_113 * x_114));
}

fn perturbNormal_mf33_vf3_f1_(cotangentFrame_1 : ptr<function, mat3x3<f32>>, textureSample : ptr<function, vec3<f32>>, scale_1 : ptr<function, f32>) -> vec3<f32> {
  var param : mat3x3<f32>;
  var param_1 : vec3<f32>;
  var param_2 : f32;
  let x_119 : vec3<f32> = *(textureSample);
  let x_125 : mat3x3<f32> = *(cotangentFrame_1);
  param = x_125;
  param_1 = ((x_119 * 2.0) - vec3<f32>(1.0, 1.0, 1.0));
  let x_128 : f32 = *(scale_1);
  param_2 = x_128;
  let x_129 : vec3<f32> = perturbNormalBase_mf33_vf3_f1_(&(param), &(param_1), &(param_2));
  return x_129;
}

fn computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(viewDirectionW : ptr<function, vec3<f32>>, vNormal : ptr<function, vec3<f32>>, lightData : ptr<function, vec4<f32>>, diffuseColor : ptr<function, vec3<f32>>, specularColor : ptr<function, vec3<f32>>, groundColor : ptr<function, vec3<f32>>, glossiness : ptr<function, f32>) -> lightingInfo {
  var ndl : f32;
  var result : lightingInfo;
  var angleW : vec3<f32>;
  var specComp : f32;
  let x_212 : vec3<f32> = *(vNormal);
  let x_213 : vec4<f32> = *(lightData);
  ndl = ((dot(x_212, vec3<f32>(x_213.x, x_213.y, x_213.z)) * 0.5) + 0.5);
  let x_220 : vec3<f32> = *(groundColor);
  let x_221 : vec3<f32> = *(diffuseColor);
  let x_222 : f32 = ndl;
  result.diffuse = mix(x_220, x_221, vec3<f32>(x_222, x_222, x_222));
  let x_227 : vec3<f32> = *(viewDirectionW);
  let x_228 : vec4<f32> = *(lightData);
  angleW = normalize((x_227 + vec3<f32>(x_228.x, x_228.y, x_228.z)));
  let x_233 : vec3<f32> = *(vNormal);
  let x_234 : vec3<f32> = angleW;
  specComp = max(0.0, dot(x_233, x_234));
  let x_237 : f32 = specComp;
  let x_238 : f32 = *(glossiness);
  specComp = pow(x_237, max(1.0, x_238));
  let x_241 : f32 = specComp;
  let x_242 : vec3<f32> = *(specularColor);
  result.specular = (x_242 * x_241);
  let x_245 : lightingInfo = result;
  return x_245;
}

fn main_1() {
  var tempTextureRead : vec4<f32>;
  var rgb : vec3<f32>;
  var output5 : vec3<f32>;
  var output4 : vec4<f32>;
  var uvOffset : vec2<f32>;
  var normalScale : f32;
  var TBNUV : vec2<f32>;
  var x_299 : vec2<f32>;
  var TBN : mat3x3<f32>;
  var param_3 : vec3<f32>;
  var param_4 : vec3<f32>;
  var param_5 : vec2<f32>;
  var param_6 : vec2<f32>;
  var invTBN : mat3x3<f32>;
  var param_7 : mat3x3<f32>;
  var parallaxLimit : f32;
  var vOffsetDir : vec2<f32>;
  var vMaxOffset : vec2<f32>;
  var numSamples : f32;
  var stepSize : f32;
  var currRayHeight : f32;
  var vCurrOffset : vec2<f32>;
  var vLastOffset : vec2<f32>;
  var lastSampledHeight : f32;
  var currSampledHeight : f32;
  var i : i32;
  var delta1 : f32;
  var delta2 : f32;
  var ratio : f32;
  var parallaxOcclusion_0 : vec2<f32>;
  var param_8 : mat3x3<f32>;
  var param_9 : vec3<f32>;
  var param_10 : f32;
  var output6 : vec2<f32>;
  var tempTextureRead1 : vec4<f32>;
  var rgb1 : vec3<f32>;
  var viewDirectionW_1 : vec3<f32>;
  var shadow : f32;
  var glossiness_1 : f32;
  var diffuseBase : vec3<f32>;
  var specularBase : vec3<f32>;
  var normalW : vec3<f32>;
  var info : lightingInfo;
  var param_11 : vec3<f32>;
  var param_12 : vec3<f32>;
  var param_13 : vec4<f32>;
  var param_14 : vec3<f32>;
  var param_15 : vec3<f32>;
  var param_16 : vec3<f32>;
  var param_17 : f32;
  var diffuseOutput : vec3<f32>;
  var specularOutput : vec3<f32>;
  var output3 : vec3<f32>;
  u_Float = 100.0;
  u_Color = vec3<f32>(0.5, 0.5, 0.5);
  let x_261 : vec2<f32> = vMainuv;
  let x_262 : vec4<f32> = textureSample(TextureSamplerTexture, TextureSamplerSampler, x_261);
  tempTextureRead = x_262;
  let x_264 : vec4<f32> = tempTextureRead;
  let x_273 : f32 = x_269.textureInfoName;
  rgb = (vec3<f32>(x_264.x, x_264.y, x_264.z) * x_273);
  let x_279 : vec3<f32> = x_269.u_cameraPosition;
  let x_282 : vec4<f32> = v_output1;
  output5 = normalize((x_279 - vec3<f32>(x_282.x, x_282.y, x_282.z)));
  output4 = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  uvOffset = vec2<f32>(0.0, 0.0);
  let x_292 : f32 = x_269.u_bumpStrength;
  normalScale = (1.0 / x_292);
  let x_298 : bool = gl_FrontFacing;
  if (x_298) {
    let x_303 : vec2<f32> = v_uv;
    x_299 = x_303;
  } else {
    let x_305 : vec2<f32> = v_uv;
    x_299 = -(x_305);
  }
  let x_307 : vec2<f32> = x_299;
  TBNUV = x_307;
  let x_310 : vec4<f32> = v_output2;
  let x_312 : f32 = normalScale;
  param_3 = (vec3<f32>(x_310.x, x_310.y, x_310.z) * x_312);
  let x_317 : vec4<f32> = v_output1;
  param_4 = vec3<f32>(x_317.x, x_317.y, x_317.z);
  let x_320 : vec2<f32> = TBNUV;
  param_5 = x_320;
  let x_324 : vec2<f32> = x_269.tangentSpaceParameter0;
  param_6 = x_324;
  let x_325 : mat3x3<f32> = cotangent_frame_vf3_vf3_vf2_vf2_(&(param_3), &(param_4), &(param_5), &(param_6));
  TBN = x_325;
  let x_328 : mat3x3<f32> = TBN;
  param_7 = x_328;
  let x_329 : mat3x3<f32> = transposeMat3_mf33_(&(param_7));
  invTBN = x_329;
  let x_331 : mat3x3<f32> = invTBN;
  let x_332 : vec3<f32> = output5;
  let x_334 : vec3<f32> = (x_331 * -(x_332));
  let x_337 : mat3x3<f32> = invTBN;
  let x_338 : vec3<f32> = output5;
  parallaxLimit = (length(vec2<f32>(x_334.x, x_334.y)) / ((x_337 * -(x_338))).z);
  let x_345 : f32 = x_269.u_parallaxScale;
  let x_346 : f32 = parallaxLimit;
  parallaxLimit = (x_346 * x_345);
  let x_349 : mat3x3<f32> = invTBN;
  let x_350 : vec3<f32> = output5;
  let x_352 : vec3<f32> = (x_349 * -(x_350));
  vOffsetDir = normalize(vec2<f32>(x_352.x, x_352.y));
  let x_356 : vec2<f32> = vOffsetDir;
  let x_357 : f32 = parallaxLimit;
  vMaxOffset = (x_356 * x_357);
  let x_361 : mat3x3<f32> = invTBN;
  let x_362 : vec3<f32> = output5;
  let x_365 : mat3x3<f32> = invTBN;
  let x_366 : vec4<f32> = v_output2;
  numSamples = (15.0 + (dot((x_361 * -(x_362)), (x_365 * vec3<f32>(x_366.x, x_366.y, x_366.z))) * -11.0));
  let x_374 : f32 = numSamples;
  stepSize = (1.0 / x_374);
  currRayHeight = 1.0;
  vCurrOffset = vec2<f32>(0.0, 0.0);
  vLastOffset = vec2<f32>(0.0, 0.0);
  lastSampledHeight = 1.0;
  currSampledHeight = 1.0;
  i = 0;
  loop {
    let x_388 : i32 = i;
    if ((x_388 < 15)) {
    } else {
      break;
    }
    let x_394 : vec2<f32> = v_uv;
    let x_395 : vec2<f32> = vCurrOffset;
    // Violates uniformity analysis:
    // let x_397 : vec4<f32> = textureSample(TextureSamplerTexture, TextureSamplerSampler, (x_394 + x_395));
    let x_397 : vec4<f32> = vec4<f32>();
    currSampledHeight = x_397.w;
    let x_400 : f32 = currSampledHeight;
    let x_401 : f32 = currRayHeight;
    if ((x_400 > x_401)) {
      let x_406 : f32 = currSampledHeight;
      let x_407 : f32 = currRayHeight;
      delta1 = (x_406 - x_407);
      let x_410 : f32 = currRayHeight;
      let x_411 : f32 = stepSize;
      let x_413 : f32 = lastSampledHeight;
      delta2 = ((x_410 + x_411) - x_413);
      let x_416 : f32 = delta1;
      let x_417 : f32 = delta1;
      let x_418 : f32 = delta2;
      ratio = (x_416 / (x_417 + x_418));
      let x_421 : f32 = ratio;
      let x_422 : vec2<f32> = vLastOffset;
      let x_424 : f32 = ratio;
      let x_426 : vec2<f32> = vCurrOffset;
      vCurrOffset = ((x_422 * x_421) + (x_426 * (1.0 - x_424)));
      break;
    } else {
      let x_431 : f32 = stepSize;
      let x_432 : f32 = currRayHeight;
      currRayHeight = (x_432 - x_431);
      let x_434 : vec2<f32> = vCurrOffset;
      vLastOffset = x_434;
      let x_435 : f32 = stepSize;
      let x_436 : vec2<f32> = vMaxOffset;
      let x_438 : vec2<f32> = vCurrOffset;
      vCurrOffset = (x_438 + (x_436 * x_435));
      let x_440 : f32 = currSampledHeight;
      lastSampledHeight = x_440;
    }

    continuing {
      let x_441 : i32 = i;
      i = (x_441 + 1);
    }
  }
  let x_444 : vec2<f32> = vCurrOffset;
  parallaxOcclusion_0 = x_444;
  let x_445 : vec2<f32> = parallaxOcclusion_0;
  uvOffset = x_445;
  let x_449 : vec2<f32> = v_uv;
  let x_450 : vec2<f32> = uvOffset;
  let x_452 : vec4<f32> = textureSample(TextureSamplerTexture, TextureSamplerSampler, (x_449 + x_450));
  let x_454 : f32 = x_269.u_bumpStrength;
  let x_457 : mat3x3<f32> = TBN;
  param_8 = x_457;
  param_9 = vec3<f32>(x_452.x, x_452.y, x_452.z);
  param_10 = (1.0 / x_454);
  let x_461 : vec3<f32> = perturbNormal_mf33_vf3_f1_(&(param_8), &(param_9), &(param_10));
  let x_462 : vec4<f32> = output4;
  output4 = vec4<f32>(x_461.x, x_461.y, x_461.z, x_462.w);
  let x_465 : vec2<f32> = v_uv;
  let x_466 : vec2<f32> = uvOffset;
  output6 = (x_465 + x_466);
  let x_474 : vec2<f32> = output6;
  let x_475 : vec4<f32> = textureSample(TextureSampler1Texture, TextureSampler1Sampler, x_474);
  tempTextureRead1 = x_475;
  let x_477 : vec4<f32> = tempTextureRead1;
  rgb1 = vec3<f32>(x_477.x, x_477.y, x_477.z);
  let x_481 : vec3<f32> = x_269.u_cameraPosition;
  let x_482 : vec4<f32> = v_output1;
  viewDirectionW_1 = normalize((x_481 - vec3<f32>(x_482.x, x_482.y, x_482.z)));
  shadow = 1.0;
  let x_488 : f32 = u_Float;
  glossiness_1 = (1.0 * x_488);
  diffuseBase = vec3<f32>(0.0, 0.0, 0.0);
  specularBase = vec3<f32>(0.0, 0.0, 0.0);
  let x_494 : vec4<f32> = output4;
  normalW = vec3<f32>(x_494.x, x_494.y, x_494.z);
  let x_501 : vec3<f32> = viewDirectionW_1;
  param_11 = x_501;
  let x_503 : vec3<f32> = normalW;
  param_12 = x_503;
  let x_507 : vec4<f32> = light0.vLightData;
  param_13 = x_507;
  let x_510 : vec4<f32> = light0.vLightDiffuse;
  param_14 = vec3<f32>(x_510.x, x_510.y, x_510.z);
  let x_514 : vec4<f32> = light0.vLightSpecular;
  param_15 = vec3<f32>(x_514.x, x_514.y, x_514.z);
  let x_518 : vec3<f32> = light0.vLightGround;
  param_16 = x_518;
  let x_520 : f32 = glossiness_1;
  param_17 = x_520;
  let x_521 : lightingInfo = computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(&(param_11), &(param_12), &(param_13), &(param_14), &(param_15), &(param_16), &(param_17));
  info = x_521;
  shadow = 1.0;
  let x_523 : vec3<f32> = info.diffuse;
  let x_524 : f32 = shadow;
  let x_526 : vec3<f32> = diffuseBase;
  diffuseBase = (x_526 + (x_523 * x_524));
  let x_529 : vec3<f32> = info.specular;
  let x_530 : f32 = shadow;
  let x_532 : vec3<f32> = specularBase;
  specularBase = (x_532 + (x_529 * x_530));
  let x_535 : vec3<f32> = diffuseBase;
  let x_536 : vec3<f32> = rgb1;
  diffuseOutput = (x_535 * x_536);
  let x_539 : vec3<f32> = specularBase;
  let x_540 : vec3<f32> = u_Color;
  specularOutput = (x_539 * x_540);
  let x_543 : vec3<f32> = diffuseOutput;
  let x_544 : vec3<f32> = specularOutput;
  output3 = (x_543 + x_544);
  let x_548 : vec3<f32> = output3;
  glFragColor = vec4<f32>(x_548.x, x_548.y, x_548.z, 1.0);
  return;
}

struct main_out {
  @location(0)
  glFragColor_1 : vec4<f32>,
};

@fragment
fn main(@location(1) vMainuv_param : vec2<f32>, @location(0) v_output1_param : vec4<f32>, @builtin(front_facing) gl_FrontFacing_param : bool, @location(3) v_uv_param : vec2<f32>, @location(2) v_output2_param : vec4<f32>) -> main_out {
  vMainuv = vMainuv_param;
  v_output1 = v_output1_param;
  gl_FrontFacing = gl_FrontFacing_param;
  v_uv = v_uv_param;
  v_output2 = v_output2_param;
  main_1();
  return main_out(glFragColor);
}
