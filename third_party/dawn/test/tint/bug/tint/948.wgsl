struct LeftOver {
  time : f32,
  @size(12)
  padding : u32,
  worldViewProjection : mat4x4<f32>,
  outputSize : vec2<f32>,
  stageSize : vec2<f32>,
  spriteMapSize : vec2<f32>,
  stageScale : f32,
  spriteCount : f32,
  colorMul : vec3<f32>,
};

@group(2) @binding(9) var<uniform> x_20 : LeftOver;

@group(2) @binding(3) var frameMapTexture : texture_2d<f32>;

@group(2) @binding(2) var frameMapSampler : sampler;

var<private> tUV : vec2<f32>;

@group(2) @binding(5) var tileMapsTexture0 : texture_2d<f32>;

@group(2) @binding(4) var tileMapsSampler : sampler;

@group(2) @binding(6) var tileMapsTexture1 : texture_2d<f32>;

@group(2) @binding(8) var animationMapTexture : texture_2d<f32>;

@group(2) @binding(7) var animationMapSampler : sampler;

var<private> mt : f32;

@group(2) @binding(1) var spriteSheetTexture : texture_2d<f32>;

@group(2) @binding(0) var spriteSheetSampler : sampler;

var<private> glFragColor : vec4<f32>;

var<private> tileID_1 : vec2<f32>;

var<private> levelUnits : vec2<f32>;

var<private> stageUnits_1 : vec2<f32>;

var<private> vPosition : vec3<f32>;

var<private> vUV : vec2<f32>;

fn getFrameData_f1_(frameID : ptr<function, f32>) -> mat4x4<f32> {
  var fX : f32;
  let x_15 : f32 = *(frameID);
  let x_25 : f32 = x_20.spriteCount;
  fX = (x_15 / x_25);
  let x_37 : f32 = fX;
  let x_40 : vec4<f32> = textureSampleBias(frameMapTexture, frameMapSampler, vec2<f32>(x_37, 0.0), 0.0);
  let x_44 : f32 = fX;
  let x_47 : vec4<f32> = textureSampleBias(frameMapTexture, frameMapSampler, vec2<f32>(x_44, 0.25), 0.0);
  let x_51 : f32 = fX;
  let x_54 : vec4<f32> = textureSampleBias(frameMapTexture, frameMapSampler, vec2<f32>(x_51, 0.5), 0.0);
  return mat4x4<f32>(vec4<f32>(x_40.x, x_40.y, x_40.z, x_40.w), vec4<f32>(x_47.x, x_47.y, x_47.z, x_47.w), vec4<f32>(x_54.x, x_54.y, x_54.z, x_54.w), vec4<f32>(vec4<f32>(0.0, 0.0, 0.0, 0.0).x, vec4<f32>(0.0, 0.0, 0.0, 0.0).y, vec4<f32>(0.0, 0.0, 0.0, 0.0).z, vec4<f32>(0.0, 0.0, 0.0, 0.0).w));
}

fn main_1() {
  var color : vec4<f32>;
  var tileUV : vec2<f32>;
  var tileID : vec2<f32>;
  var sheetUnits : vec2<f32>;
  var spriteUnits : f32;
  var stageUnits : vec2<f32>;
  var i : i32;
  var frameID_1 : f32;
  var animationData : vec4<f32>;
  var f : f32;
  var frameData : mat4x4<f32>;
  var param : f32;
  var frameSize : vec2<f32>;
  var offset_1 : vec2<f32>;
  var ratio : vec2<f32>;
  var nc : vec4<f32>;
  var alpha : f32;
  var mixed : vec3<f32>;
  color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  let x_86 : vec2<f32> = tUV;
  tileUV = fract(x_86);
  let x_91 : f32 = tileUV.y;
  tileUV.y = (1.0 - x_91);
  let x_95 : vec2<f32> = tUV;
  tileID = floor(x_95);
  let x_101 : vec2<f32> = x_20.spriteMapSize;
  sheetUnits = (vec2<f32>(1.0, 1.0) / x_101);
  let x_106 : f32 = x_20.spriteCount;
  spriteUnits = (1.0 / x_106);
  let x_111 : vec2<f32> = x_20.stageSize;
  stageUnits = (vec2<f32>(1.0, 1.0) / x_111);
  i = 0;
  loop {
    let x_122 : i32 = i;
    if ((x_122 < 2)) {
    } else {
      break;
    }
    let x_126 : i32 = i;
    switch(x_126) {
      case 1: {
        let x_150 : vec2<f32> = tileID;
        let x_154 : vec2<f32> = x_20.stageSize;
        let x_156 : vec4<f32> = textureSampleBias(tileMapsTexture1, tileMapsSampler, ((x_150 + vec2<f32>(0.5, 0.5)) / x_154), 0.0);
        frameID_1 = x_156.x;
      }
      case 0: {
        let x_136 : vec2<f32> = tileID;
        let x_140 : vec2<f32> = x_20.stageSize;
        let x_142 : vec4<f32> = textureSampleBias(tileMapsTexture0, tileMapsSampler, ((x_136 + vec2<f32>(0.5, 0.5)) / x_140), 0.0);
        frameID_1 = x_142.x;
      }
      default: {
      }
    }
    let x_166 : f32 = frameID_1;
    let x_169 : f32 = x_20.spriteCount;
    let x_172 : vec4<f32> = textureSampleBias(animationMapTexture, animationMapSampler, vec2<f32>(((x_166 + 0.5) / x_169), 0.0), 0.0);
    animationData = x_172;
    let x_174 : f32 = animationData.y;
    if ((x_174 > 0.0)) {
      let x_181 : f32 = x_20.time;
      let x_184 : f32 = animationData.z;
      mt = ((x_181 * x_184) % 1.0);
      f = 0.0;
      loop {
        let x_193 : f32 = f;
        if ((x_193 < 8.0)) {
        } else {
          break;
        }
        let x_197 : f32 = animationData.y;
        let x_198 : f32 = mt;
        if ((x_197 > x_198)) {
          let x_203 : f32 = animationData.x;
          frameID_1 = x_203;
          break;
        }
        let x_208 : f32 = frameID_1;
        let x_211 : f32 = x_20.spriteCount;
        let x_214 : f32 = f;
        // Violates uniformity analysis:
        // let x_217 : vec4<f32> = textureSampleBias(animationMapTexture, animationMapSampler, vec2<f32>(((x_208 + 0.5) / x_211), (0.125 * x_214)), 0.0);
        let x_217 : vec4<f32> = vec4<f32>(0);
        animationData = x_217;

        continuing {
          let x_218 : f32 = f;
          f = (x_218 + 1.0);
        }
      }
    }
    let x_222 : f32 = frameID_1;
    param = (x_222 + 0.5);
    let x_225 : mat4x4<f32> = getFrameData_f1_(&(param));
    frameData = x_225;
    let x_228 : vec4<f32> = frameData[0];
    let x_231 : vec2<f32> = x_20.spriteMapSize;
    frameSize = (vec2<f32>(x_228.w, x_228.z) / x_231);
    let x_235 : vec4<f32> = frameData[0];
    let x_237 : vec2<f32> = sheetUnits;
    offset_1 = (vec2<f32>(x_235.x, x_235.y) * x_237);
    let x_241 : vec4<f32> = frameData[2];
    let x_244 : vec4<f32> = frameData[0];
    ratio = (vec2<f32>(x_241.x, x_241.y) / vec2<f32>(x_244.w, x_244.z));
    let x_248 : f32 = frameData[2].z;
    if ((x_248 == 1.0)) {
      let x_252 : vec2<f32> = tileUV;
      tileUV = vec2<f32>(x_252.y, x_252.x);
    }
    let x_254 : i32 = i;
    if ((x_254 == 0)) {
      let x_263 : vec2<f32> = tileUV;
      let x_264 : vec2<f32> = frameSize;
      let x_266 : vec2<f32> = offset_1;
      let x_268 : vec4<f32> = textureSample(spriteSheetTexture, spriteSheetSampler, ((x_263 * x_264) + x_266));
      color = x_268;
    } else {
      let x_274 : vec2<f32> = tileUV;
      let x_275 : vec2<f32> = frameSize;
      let x_277 : vec2<f32> = offset_1;
      let x_279 : vec4<f32> = textureSample(spriteSheetTexture, spriteSheetSampler, ((x_274 * x_275) + x_277));
      nc = x_279;
      let x_283 : f32 = color.w;
      let x_285 : f32 = nc.w;
      alpha = min((x_283 + x_285), 1.0);
      let x_290 : vec4<f32> = color;
      let x_292 : vec4<f32> = nc;
      let x_295 : f32 = nc.w;
      mixed = mix(vec3<f32>(x_290.x, x_290.y, x_290.z), vec3<f32>(x_292.x, x_292.y, x_292.z), vec3<f32>(x_295, x_295, x_295));
      let x_298 : vec3<f32> = mixed;
      let x_299 : f32 = alpha;
      color = vec4<f32>(x_298.x, x_298.y, x_298.z, x_299);
    }

    continuing {
      let x_304 : i32 = i;
      i = (x_304 + 1);
    }
  }
  let x_310 : vec3<f32> = x_20.colorMul;
  let x_311 : vec4<f32> = color;
  let x_313 : vec3<f32> = (vec3<f32>(x_311.x, x_311.y, x_311.z) * x_310);
  let x_314 : vec4<f32> = color;
  color = vec4<f32>(x_313.x, x_313.y, x_313.z, x_314.w);
  let x_318 : vec4<f32> = color;
  glFragColor = x_318;
  return;
}

struct main_out {
  @location(0)
  glFragColor_1 : vec4<f32>,
};

@fragment
fn main(@location(2) tUV_param : vec2<f32>, @location(5) tileID_1_param : vec2<f32>, @location(4) levelUnits_param : vec2<f32>, @location(3) stageUnits_1_param : vec2<f32>, @location(0) vPosition_param : vec3<f32>, @location(1) vUV_param : vec2<f32>) -> main_out {
  tUV = tUV_param;
  tileID_1 = tileID_1_param;
  levelUnits = levelUnits_param;
  stageUnits_1 = stageUnits_1_param;
  vPosition = vPosition_param;
  vUV = vUV_param;
  main_1();
  return main_out(glFragColor);
}
