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

fn main_1() {
  var x_90 : i32;
  var x_91 : i32;
  var x_92 : i32;
  var x_93 : i32;
  var x_94 : i32;
  var x_95 : i32;
  var x_96 : i32;
  var x_97 : i32;
  var x_98 : i32;
  var x_99 : i32;
  var x_100 : i32;
  var x_101 : i32;
  var x_102 : i32;
  var x_103 : array<i32, 10u>;
  var x_104 : i32;
  var x_105 : i32;
  var x_106 : i32;
  var i_2 : i32;
  var uv : vec2<f32>;
  var color : vec3<f32>;
  let x_107 : vec4<f32> = x_GLF_pos;
  x_GLF_FragCoord = ((x_107 + vec4<f32>(1.0, 1.0, 0.0, 0.0)) * vec4<f32>(128.0, 128.0, 1.0, 1.0));
  i_2 = 0;
  loop {
    let x_114 : i32 = i_2;
    if ((x_114 < 10)) {
    } else {
      break;
    }
    let x_117 : i32 = i_2;
    let x_118 : i32 = i_2;
    obj.numbers[x_117] = (10 - x_118);
    let x_121 : i32 = i_2;
    let x_122 : i32 = i_2;
    let x_124 : i32 = obj.numbers[x_122];
    let x_125 : i32 = i_2;
    let x_127 : i32 = obj.numbers[x_125];
    obj.numbers[x_121] = (x_124 * x_127);

    continuing {
      let x_130 : i32 = i_2;
      i_2 = (x_130 + 1);
    }
  }
  x_100 = 0;
  x_101 = 9;
  x_102 = -1;
  let x_132 : i32 = x_102;
  let x_133 : i32 = (x_132 + 1);
  x_102 = x_133;
  let x_134 : i32 = x_100;
  x_103[x_133] = x_134;
  let x_136 : i32 = x_102;
  let x_137 : i32 = (x_136 + 1);
  x_102 = x_137;
  let x_138 : i32 = x_101;
  x_103[x_137] = x_138;
  loop {
    let x_144 : i32 = x_102;
    if ((x_144 >= 0)) {
    } else {
      break;
    }
    let x_147 : i32 = x_102;
    x_102 = (x_147 - 1);
    let x_150 : i32 = x_103[x_147];
    x_101 = x_150;
    let x_151 : i32 = x_102;
    x_102 = (x_151 - 1);
    let x_154 : i32 = x_103[x_151];
    x_100 = x_154;
    let x_155 : i32 = x_100;
    x_105 = x_155;
    let x_156 : i32 = x_101;
    x_106 = x_156;
    let x_157 : i32 = x_106;
    let x_159 : i32 = obj.numbers[x_157];
    x_92 = x_159;
    let x_160 : i32 = x_105;
    x_93 = (x_160 - 1);
    let x_162 : i32 = x_105;
    x_94 = x_162;
    loop {
      let x_167 : i32 = x_94;
      let x_168 : i32 = x_106;
      if ((x_167 <= (x_168 - 1))) {
      } else {
        break;
      }
      let x_172 : i32 = x_94;
      let x_174 : i32 = obj.numbers[x_172];
      let x_175 : i32 = x_92;
      if ((x_174 <= x_175)) {
        let x_179 : i32 = x_93;
        x_93 = (x_179 + 1);
        let x_181 : i32 = x_93;
        x_95 = x_181;
        let x_182 : i32 = x_94;
        x_96 = x_182;
        let x_183 : i32 = x_95;
        let x_185 : i32 = obj.numbers[x_183];
        x_91 = x_185;
        let x_186 : i32 = x_95;
        let x_187 : i32 = x_96;
        let x_189 : i32 = obj.numbers[x_187];
        obj.numbers[x_186] = x_189;
        let x_191 : i32 = x_96;
        let x_192 : i32 = x_91;
        obj.numbers[x_191] = x_192;
      }

      continuing {
        let x_194 : i32 = x_94;
        x_94 = (x_194 + 1);
      }
    }
    let x_196 : i32 = x_93;
    x_97 = (x_196 + 1);
    let x_198 : i32 = x_106;
    x_98 = x_198;
    let x_199 : i32 = x_97;
    let x_201 : i32 = obj.numbers[x_199];
    x_90 = x_201;
    let x_202 : i32 = x_97;
    let x_203 : i32 = x_98;
    let x_205 : i32 = obj.numbers[x_203];
    obj.numbers[x_202] = x_205;
    let x_207 : i32 = x_98;
    let x_208 : i32 = x_90;
    obj.numbers[x_207] = x_208;
    let x_210 : i32 = x_93;
    x_99 = (x_210 + 1);
    let x_212 : i32 = x_99;
    x_104 = x_212;
    let x_213 : i32 = x_104;
    let x_215 : i32 = x_100;
    if (((x_213 - 1) > x_215)) {
      let x_219 : i32 = x_102;
      let x_220 : i32 = (x_219 + 1);
      x_102 = x_220;
      let x_221 : i32 = x_100;
      x_103[x_220] = x_221;
      let x_223 : i32 = x_102;
      let x_224 : i32 = (x_223 + 1);
      x_102 = x_224;
      let x_225 : i32 = x_104;
      x_103[x_224] = (x_225 - 1);
    }
    let x_228 : i32 = x_104;
    let x_230 : i32 = x_101;
    if (((x_228 + 1) < x_230)) {
      let x_234 : i32 = x_102;
      let x_235 : i32 = (x_234 + 1);
      x_102 = x_235;
      let x_236 : i32 = x_104;
      x_103[x_235] = (x_236 + 1);
      let x_239 : i32 = x_102;
      let x_240 : i32 = (x_239 + 1);
      x_102 = x_240;
      let x_241 : i32 = x_101;
      x_103[x_240] = x_241;
    }
  }
  let x_243 : vec4<f32> = x_GLF_FragCoord;
  let x_246 : vec2<f32> = x_34.resolution;
  uv = (vec2<f32>(x_243.x, x_243.y) / x_246);
  color = vec3<f32>(1.0, 2.0, 3.0);
  let x_249 : i32 = obj.numbers[0];
  let x_252 : f32 = color.x;
  color.x = (x_252 + f32(x_249));
  let x_256 : f32 = uv.x;
  if ((x_256 > 0.25)) {
    let x_261 : i32 = obj.numbers[1];
    let x_264 : f32 = color.x;
    color.x = (x_264 + f32(x_261));
  }
  let x_268 : f32 = uv.x;
  if ((x_268 > 0.5)) {
    let x_273 : i32 = obj.numbers[2];
    let x_276 : f32 = color.y;
    color.y = (x_276 + f32(x_273));
  }
  let x_280 : f32 = uv.x;
  if ((x_280 > 0.75)) {
    let x_285 : i32 = obj.numbers[3];
    let x_288 : f32 = color.z;
    color.z = (x_288 + f32(x_285));
  }
  let x_292 : i32 = obj.numbers[4];
  let x_295 : f32 = color.y;
  color.y = (x_295 + f32(x_292));
  let x_299 : f32 = uv.y;
  if ((x_299 > 0.25)) {
    let x_304 : i32 = obj.numbers[5];
    let x_307 : f32 = color.x;
    color.x = (x_307 + f32(x_304));
  }
  let x_311 : f32 = uv.y;
  if ((x_311 > 0.5)) {
    let x_316 : i32 = obj.numbers[6];
    let x_319 : f32 = color.y;
    color.y = (x_319 + f32(x_316));
  }
  let x_323 : f32 = uv.y;
  if ((x_323 > 0.75)) {
    let x_328 : i32 = obj.numbers[7];
    let x_331 : f32 = color.z;
    color.z = (x_331 + f32(x_328));
  }
  let x_335 : i32 = obj.numbers[8];
  let x_338 : f32 = color.z;
  color.z = (x_338 + f32(x_335));
  let x_342 : f32 = uv.x;
  let x_344 : f32 = uv.y;
  if ((abs((x_342 - x_344)) < 0.25)) {
    let x_351 : i32 = obj.numbers[9];
    let x_354 : f32 = color.x;
    color.x = (x_354 + f32(x_351));
  }
  let x_357 : vec3<f32> = color;
  let x_358 : vec3<f32> = normalize(x_357);
  frag_color = vec4<f32>(x_358.x, x_358.y, x_358.z, 1.0);
  let x_363 : vec4<f32> = x_GLF_pos;
  gl_Position = x_363;
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
  let x_366 : i32 = *(i);
  let x_368 : i32 = obj.numbers[x_366];
  temp = x_368;
  let x_369 : i32 = *(i);
  let x_370 : i32 = *(j);
  let x_372 : i32 = obj.numbers[x_370];
  obj.numbers[x_369] = x_372;
  let x_374 : i32 = *(j);
  let x_375 : i32 = temp;
  obj.numbers[x_374] = x_375;
  return;
}

fn performPartition_i1_i1_(l : ptr<function, i32>, h : ptr<function, i32>) -> i32 {
  var pivot : i32;
  var i_1 : i32;
  var j_1 : i32;
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  let x_378 : i32 = *(h);
  let x_380 : i32 = obj.numbers[x_378];
  pivot = x_380;
  let x_381 : i32 = *(l);
  i_1 = (x_381 - 1);
  let x_383 : i32 = *(l);
  j_1 = x_383;
  loop {
    let x_388 : i32 = j_1;
    let x_389 : i32 = *(h);
    if ((x_388 <= (x_389 - 1))) {
    } else {
      break;
    }
    let x_393 : i32 = j_1;
    let x_395 : i32 = obj.numbers[x_393];
    let x_396 : i32 = pivot;
    if ((x_395 <= x_396)) {
      let x_400 : i32 = i_1;
      i_1 = (x_400 + 1);
      let x_402 : i32 = i_1;
      param = x_402;
      let x_403 : i32 = j_1;
      param_1 = x_403;
      swap_i1_i1_(&(param), &(param_1));
    }

    continuing {
      let x_405 : i32 = j_1;
      j_1 = (x_405 + 1);
    }
  }
  let x_407 : i32 = i_1;
  param_2 = (x_407 + 1);
  let x_409 : i32 = *(h);
  param_3 = x_409;
  swap_i1_i1_(&(param_2), &(param_3));
  let x_411 : i32 = i_1;
  return (x_411 + 1);
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
  let x_414 : i32 = top;
  let x_415 : i32 = (x_414 + 1);
  top = x_415;
  let x_416 : i32 = l_1;
  stack[x_415] = x_416;
  let x_418 : i32 = top;
  let x_419 : i32 = (x_418 + 1);
  top = x_419;
  let x_420 : i32 = h_1;
  stack[x_419] = x_420;
  loop {
    let x_426 : i32 = top;
    if ((x_426 >= 0)) {
    } else {
      break;
    }
    let x_429 : i32 = top;
    top = (x_429 - 1);
    let x_432 : i32 = stack[x_429];
    h_1 = x_432;
    let x_433 : i32 = top;
    top = (x_433 - 1);
    let x_436 : i32 = stack[x_433];
    l_1 = x_436;
    let x_437 : i32 = l_1;
    param_4 = x_437;
    let x_438 : i32 = h_1;
    param_5 = x_438;
    let x_439 : i32 = performPartition_i1_i1_(&(param_4), &(param_5));
    p = x_439;
    let x_440 : i32 = p;
    let x_442 : i32 = l_1;
    if (((x_440 - 1) > x_442)) {
      let x_446 : i32 = top;
      let x_447 : i32 = (x_446 + 1);
      top = x_447;
      let x_448 : i32 = l_1;
      stack[x_447] = x_448;
      let x_450 : i32 = top;
      let x_451 : i32 = (x_450 + 1);
      top = x_451;
      let x_452 : i32 = p;
      stack[x_451] = (x_452 - 1);
    }
    let x_455 : i32 = p;
    let x_457 : i32 = h_1;
    if (((x_455 + 1) < x_457)) {
      let x_461 : i32 = top;
      let x_462 : i32 = (x_461 + 1);
      top = x_462;
      let x_463 : i32 = p;
      stack[x_462] = (x_463 + 1);
      let x_466 : i32 = top;
      let x_467 : i32 = (x_466 + 1);
      top = x_467;
      let x_468 : i32 = h_1;
      stack[x_467] = x_468;
    }
  }
  return;
}
