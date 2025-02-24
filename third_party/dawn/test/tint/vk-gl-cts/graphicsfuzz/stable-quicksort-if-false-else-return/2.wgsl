struct QuicksortObject {
  numbers : array<i32, 10u>,
}

struct buf0 {
  resolution : vec2<f32>,
}

var<private> obj : QuicksortObject;

var<private> x_GLF_FragCoord : vec4<f32>;

var<private> x_GLF_pos : vec4<f32>;

@group(0) @binding(0) var<uniform> x_34 : buf0;

var<private> frag_color : vec4<f32>;

var<private> gl_Position : vec4<f32>;

fn performPartition_i1_i1_(l : ptr<function, i32>, h : ptr<function, i32>) -> i32 {
  var x_314 : i32;
  var x_315 : i32;
  var pivot : i32;
  var i_1 : i32;
  var j_1 : i32;
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  let x_316 : i32 = *(h);
  let x_318 : i32 = obj.numbers[x_316];
  pivot = x_318;
  let x_319 : i32 = *(l);
  i_1 = (x_319 - 1);
  let x_321 : i32 = *(l);
  j_1 = x_321;
  loop {
    let x_326 : i32 = j_1;
    let x_327 : i32 = *(h);
    if ((x_326 <= (x_327 - 1))) {
    } else {
      break;
    }
    let x_331 : i32 = j_1;
    let x_333 : i32 = obj.numbers[x_331];
    let x_334 : i32 = pivot;
    if ((x_333 <= x_334)) {
      let x_338 : i32 = i_1;
      i_1 = (x_338 + 1);
      let x_340 : i32 = i_1;
      param = x_340;
      let x_341 : i32 = j_1;
      param_1 = x_341;
      let x_342 : i32 = param;
      let x_344 : i32 = obj.numbers[x_342];
      x_315 = x_344;
      let x_345 : i32 = param;
      let x_346 : i32 = param_1;
      let x_348 : i32 = obj.numbers[x_346];
      obj.numbers[x_345] = x_348;
      let x_350 : i32 = param_1;
      let x_351 : i32 = x_315;
      obj.numbers[x_350] = x_351;
    }

    continuing {
      let x_353 : i32 = j_1;
      j_1 = (x_353 + 1);
    }
  }
  let x_355 : i32 = i_1;
  param_2 = (x_355 + 1);
  let x_357 : i32 = *(h);
  param_3 = x_357;
  let x_358 : i32 = param_2;
  let x_360 : i32 = obj.numbers[x_358];
  x_314 = x_360;
  let x_361 : i32 = param_2;
  let x_362 : i32 = param_3;
  let x_364 : i32 = obj.numbers[x_362];
  obj.numbers[x_361] = x_364;
  let x_366 : i32 = param_3;
  let x_367 : i32 = x_314;
  obj.numbers[x_366] = x_367;
  if (false) {
  } else {
    let x_372 : i32 = i_1;
    return (x_372 + 1);
  }
  return 0;
}

fn main_1() {
  var x_91 : i32;
  var x_92 : i32;
  var x_93 : i32;
  var x_94 : array<i32, 10u>;
  var x_95 : i32;
  var x_96 : i32;
  var x_97 : i32;
  var i_2 : i32;
  var uv : vec2<f32>;
  var color : vec3<f32>;
  let x_98 : vec4<f32> = x_GLF_pos;
  x_GLF_FragCoord = ((x_98 + vec4<f32>(1.0, 1.0, 0.0, 0.0)) * vec4<f32>(128.0, 128.0, 1.0, 1.0));
  i_2 = 0;
  loop {
    let x_105 : i32 = i_2;
    if ((x_105 < 10)) {
    } else {
      break;
    }
    let x_108 : i32 = i_2;
    let x_109 : i32 = i_2;
    obj.numbers[x_108] = (10 - x_109);
    let x_112 : i32 = i_2;
    let x_113 : i32 = i_2;
    let x_115 : i32 = obj.numbers[x_113];
    let x_116 : i32 = i_2;
    let x_118 : i32 = obj.numbers[x_116];
    obj.numbers[x_112] = (x_115 * x_118);

    continuing {
      let x_121 : i32 = i_2;
      i_2 = (x_121 + 1);
    }
  }
  x_91 = 0;
  x_92 = 9;
  x_93 = -1;
  let x_123 : i32 = x_93;
  let x_124 : i32 = (x_123 + 1);
  x_93 = x_124;
  let x_125 : i32 = x_91;
  x_94[x_124] = x_125;
  let x_127 : i32 = x_93;
  let x_128 : i32 = (x_127 + 1);
  x_93 = x_128;
  let x_129 : i32 = x_92;
  x_94[x_128] = x_129;
  loop {
    let x_135 : i32 = x_93;
    if ((x_135 >= 0)) {
    } else {
      break;
    }
    let x_138 : i32 = x_93;
    x_93 = (x_138 - 1);
    let x_141 : i32 = x_94[x_138];
    x_92 = x_141;
    let x_142 : i32 = x_93;
    x_93 = (x_142 - 1);
    let x_145 : i32 = x_94[x_142];
    x_91 = x_145;
    let x_146 : i32 = x_91;
    x_96 = x_146;
    let x_147 : i32 = x_92;
    x_97 = x_147;
    let x_148 : i32 = performPartition_i1_i1_(&(x_96), &(x_97));
    x_95 = x_148;
    let x_149 : i32 = x_95;
    let x_151 : i32 = x_91;
    if (((x_149 - 1) > x_151)) {
      let x_155 : i32 = x_93;
      let x_156 : i32 = (x_155 + 1);
      x_93 = x_156;
      let x_157 : i32 = x_91;
      x_94[x_156] = x_157;
      let x_159 : i32 = x_93;
      let x_160 : i32 = (x_159 + 1);
      x_93 = x_160;
      let x_161 : i32 = x_95;
      x_94[x_160] = (x_161 - 1);
    }
    let x_164 : i32 = x_95;
    let x_166 : i32 = x_92;
    if (((x_164 + 1) < x_166)) {
      let x_170 : i32 = x_93;
      let x_171 : i32 = (x_170 + 1);
      x_93 = x_171;
      let x_172 : i32 = x_95;
      x_94[x_171] = (x_172 + 1);
      let x_175 : i32 = x_93;
      let x_176 : i32 = (x_175 + 1);
      x_93 = x_176;
      let x_177 : i32 = x_92;
      x_94[x_176] = x_177;
    }
  }
  let x_179 : vec4<f32> = x_GLF_FragCoord;
  let x_182 : vec2<f32> = x_34.resolution;
  uv = (vec2<f32>(x_179.x, x_179.y) / x_182);
  color = vec3<f32>(1.0, 2.0, 3.0);
  let x_185 : i32 = obj.numbers[0];
  let x_188 : f32 = color.x;
  color.x = (x_188 + f32(x_185));
  let x_192 : f32 = uv.x;
  if ((x_192 > 0.25)) {
    let x_197 : i32 = obj.numbers[1];
    let x_200 : f32 = color.x;
    color.x = (x_200 + f32(x_197));
  }
  let x_204 : f32 = uv.x;
  if ((x_204 > 0.5)) {
    let x_209 : i32 = obj.numbers[2];
    let x_212 : f32 = color.y;
    color.y = (x_212 + f32(x_209));
  }
  let x_216 : f32 = uv.x;
  if ((x_216 > 0.75)) {
    let x_221 : i32 = obj.numbers[3];
    let x_224 : f32 = color.z;
    color.z = (x_224 + f32(x_221));
  }
  let x_228 : i32 = obj.numbers[4];
  let x_231 : f32 = color.y;
  color.y = (x_231 + f32(x_228));
  let x_235 : f32 = uv.y;
  if ((x_235 > 0.25)) {
    let x_240 : i32 = obj.numbers[5];
    let x_243 : f32 = color.x;
    color.x = (x_243 + f32(x_240));
  }
  let x_247 : f32 = uv.y;
  if ((x_247 > 0.5)) {
    let x_252 : i32 = obj.numbers[6];
    let x_255 : f32 = color.y;
    color.y = (x_255 + f32(x_252));
  }
  let x_259 : f32 = uv.y;
  if ((x_259 > 0.75)) {
    let x_264 : i32 = obj.numbers[7];
    let x_267 : f32 = color.z;
    color.z = (x_267 + f32(x_264));
  }
  let x_271 : i32 = obj.numbers[8];
  let x_274 : f32 = color.z;
  color.z = (x_274 + f32(x_271));
  let x_278 : f32 = uv.x;
  let x_280 : f32 = uv.y;
  if ((abs((x_278 - x_280)) < 0.25)) {
    let x_287 : i32 = obj.numbers[9];
    let x_290 : f32 = color.x;
    color.x = (x_290 + f32(x_287));
  }
  let x_293 : vec3<f32> = color;
  let x_294 : vec3<f32> = normalize(x_293);
  frag_color = vec4<f32>(x_294.x, x_294.y, x_294.z, 1.0);
  let x_299 : vec4<f32> = x_GLF_pos;
  gl_Position = x_299;
  return;
}

struct main_out {
  @location(0)
  frag_color_1 : vec4<f32>,
  @builtin(position)
  gl_Position : vec4<f32>,
}

@vertex
fn main(@location(0) x_GLF_pos_param : vec4<f32>) -> main_out {
  x_GLF_pos = x_GLF_pos_param;
  main_1();
  return main_out(frag_color, gl_Position);
}

fn swap_i1_i1_(i : ptr<function, i32>, j : ptr<function, i32>) {
  var temp : i32;
  let x_302 : i32 = *(i);
  let x_304 : i32 = obj.numbers[x_302];
  temp = x_304;
  let x_305 : i32 = *(i);
  let x_306 : i32 = *(j);
  let x_308 : i32 = obj.numbers[x_306];
  obj.numbers[x_305] = x_308;
  let x_310 : i32 = *(j);
  let x_311 : i32 = temp;
  obj.numbers[x_310] = x_311;
  return;
}

fn quicksort_() {
  var l_1 : i32;
  var h_1 : i32;
  var top : i32;
  var stack : array<i32, 10u>;
  var p : i32;
  var param_4 : i32;
  var param_5 : i32;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  let x_376 : i32 = top;
  let x_377 : i32 = (x_376 + 1);
  top = x_377;
  let x_378 : i32 = l_1;
  stack[x_377] = x_378;
  let x_380 : i32 = top;
  let x_381 : i32 = (x_380 + 1);
  top = x_381;
  let x_382 : i32 = h_1;
  stack[x_381] = x_382;
  loop {
    let x_388 : i32 = top;
    if ((x_388 >= 0)) {
    } else {
      break;
    }
    let x_391 : i32 = top;
    top = (x_391 - 1);
    let x_394 : i32 = stack[x_391];
    h_1 = x_394;
    let x_395 : i32 = top;
    top = (x_395 - 1);
    let x_398 : i32 = stack[x_395];
    l_1 = x_398;
    let x_399 : i32 = l_1;
    param_4 = x_399;
    let x_400 : i32 = h_1;
    param_5 = x_400;
    let x_401 : i32 = performPartition_i1_i1_(&(param_4), &(param_5));
    p = x_401;
    let x_402 : i32 = p;
    let x_404 : i32 = l_1;
    if (((x_402 - 1) > x_404)) {
      let x_408 : i32 = top;
      let x_409 : i32 = (x_408 + 1);
      top = x_409;
      let x_410 : i32 = l_1;
      stack[x_409] = x_410;
      let x_412 : i32 = top;
      let x_413 : i32 = (x_412 + 1);
      top = x_413;
      let x_414 : i32 = p;
      stack[x_413] = (x_414 - 1);
    }
    let x_417 : i32 = p;
    let x_419 : i32 = h_1;
    if (((x_417 + 1) < x_419)) {
      let x_423 : i32 = top;
      let x_424 : i32 = (x_423 + 1);
      top = x_424;
      let x_425 : i32 = p;
      stack[x_424] = (x_425 + 1);
      let x_428 : i32 = top;
      let x_429 : i32 = (x_428 + 1);
      top = x_429;
      let x_430 : i32 = h_1;
      stack[x_429] = x_430;
    }
  }
  return;
}
