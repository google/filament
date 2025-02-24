struct QuicksortObject {
  numbers : array<i32, 10u>,
}

struct buf0 {
  resolution : vec2<f32>,
}

var<private> obj : QuicksortObject;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_34 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn swap_i1_i1_(i : ptr<function, i32>, j : ptr<function, i32>) {
  var temp : i32;
  let x_230 : i32 = *(i);
  let x_232 : i32 = obj.numbers[x_230];
  temp = x_232;
  let x_233 : i32 = *(i);
  let x_234 : i32 = *(j);
  let x_236 : i32 = obj.numbers[x_234];
  obj.numbers[x_233] = x_236;
  let x_238 : i32 = *(j);
  let x_239 : i32 = temp;
  obj.numbers[x_238] = x_239;
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
  let x_242 : i32 = *(h);
  let x_244 : i32 = obj.numbers[x_242];
  pivot = x_244;
  let x_245 : i32 = *(l);
  i_1 = (x_245 - 1);
  let x_247 : i32 = *(l);
  j_1 = x_247;
  loop {
    let x_252 : i32 = j_1;
    let x_253 : i32 = *(h);
    if ((x_252 <= (x_253 - 1))) {
    } else {
      break;
    }
    let x_257 : i32 = j_1;
    let x_259 : i32 = obj.numbers[x_257];
    let x_260 : i32 = pivot;
    if ((x_259 <= x_260)) {
      let x_264 : i32 = i_1;
      i_1 = (x_264 + 1);
      let x_266 : i32 = i_1;
      param = x_266;
      let x_267 : i32 = j_1;
      param_1 = x_267;
      swap_i1_i1_(&(param), &(param_1));
    }

    continuing {
      let x_269 : i32 = j_1;
      j_1 = (x_269 + 1);
    }
  }
  let x_271 : i32 = i_1;
  i_1 = (x_271 + 1);
  let x_273 : i32 = i_1;
  param_2 = x_273;
  let x_274 : i32 = *(h);
  param_3 = x_274;
  swap_i1_i1_(&(param_2), &(param_3));
  let x_276 : i32 = i_1;
  return x_276;
}

fn quicksort_() {
  var l_1 : i32;
  var h_1 : i32;
  var top : i32;
  var stack : array<i32, 10u>;
  var int_a : i32;
  var x_278 : i32;
  var x_279 : i32;
  var clamp_a : i32;
  var p : i32;
  var param_4 : i32;
  var param_5 : i32;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  let x_280 : i32 = top;
  let x_281 : i32 = (x_280 + 1);
  top = x_281;
  let x_282 : i32 = l_1;
  stack[x_281] = x_282;
  let x_285 : f32 = gl_FragCoord.y;
  if ((x_285 >= 0.0)) {
    let x_290 : i32 = h_1;
    if (false) {
      x_279 = 1;
    } else {
      let x_294 : i32 = h_1;
      x_279 = (x_294 << bitcast<u32>(0));
    }
    let x_296 : i32 = x_279;
    x_278 = (x_290 | x_296);
  } else {
    x_278 = 1;
  }
  let x_298 : i32 = x_278;
  int_a = x_298;
  let x_299 : i32 = h_1;
  let x_300 : i32 = h_1;
  let x_301 : i32 = int_a;
  clamp_a = clamp(x_299, x_300, x_301);
  let x_303 : i32 = top;
  let x_304 : i32 = (x_303 + 1);
  top = x_304;
  let x_305 : i32 = clamp_a;
  stack[x_304] = (x_305 / 1);
  loop {
    let x_312 : i32 = top;
    if ((x_312 >= 0)) {
    } else {
      break;
    }
    let x_315 : i32 = top;
    top = (x_315 - 1);
    let x_318 : i32 = stack[x_315];
    h_1 = x_318;
    let x_319 : i32 = top;
    top = (x_319 - 1);
    let x_322 : i32 = stack[x_319];
    l_1 = x_322;
    let x_323 : i32 = l_1;
    param_4 = x_323;
    let x_324 : i32 = h_1;
    param_5 = x_324;
    let x_325 : i32 = performPartition_i1_i1_(&(param_4), &(param_5));
    p = x_325;
    let x_326 : i32 = p;
    let x_328 : i32 = l_1;
    if (((x_326 - 1) > x_328)) {
      let x_332 : i32 = top;
      let x_333 : i32 = (x_332 + 1);
      top = x_333;
      let x_334 : i32 = l_1;
      stack[x_333] = x_334;
      let x_336 : i32 = top;
      let x_337 : i32 = (x_336 + 1);
      top = x_337;
      let x_338 : i32 = p;
      stack[x_337] = (x_338 - 1);
    }
    let x_341 : i32 = p;
    let x_343 : i32 = h_1;
    if (((x_341 + 1) < x_343)) {
      let x_347 : i32 = top;
      let x_348 : i32 = (x_347 + 1);
      top = x_348;
      let x_349 : i32 = p;
      stack[x_348] = (x_349 + 1);
      let x_352 : i32 = top;
      let x_353 : i32 = (x_352 + 1);
      top = x_353;
      let x_354 : i32 = h_1;
      stack[x_353] = x_354;
    }
  }
  return;
}

fn main_1() {
  var i_2 : i32;
  var uv : vec2<f32>;
  var color : vec3<f32>;
  i_2 = 0;
  loop {
    let x_90 : i32 = i_2;
    if ((x_90 < 10)) {
    } else {
      break;
    }
    let x_93 : i32 = i_2;
    let x_94 : i32 = i_2;
    obj.numbers[x_93] = (10 - x_94);
    let x_97 : i32 = i_2;
    let x_98 : i32 = i_2;
    let x_100 : i32 = obj.numbers[x_98];
    let x_101 : i32 = i_2;
    let x_103 : i32 = obj.numbers[x_101];
    obj.numbers[x_97] = (x_100 * x_103);

    continuing {
      let x_106 : i32 = i_2;
      i_2 = (x_106 + 1);
    }
  }
  quicksort_();
  let x_109 : vec4<f32> = gl_FragCoord;
  let x_112 : vec2<f32> = x_34.resolution;
  uv = (vec2<f32>(x_109.x, x_109.y) / x_112);
  color = vec3<f32>(1.0, 2.0, 3.0);
  let x_115 : i32 = obj.numbers[0];
  let x_118 : f32 = color.x;
  color.x = (x_118 + f32(x_115));
  let x_122 : f32 = uv.x;
  if ((x_122 > 0.25)) {
    let x_127 : i32 = obj.numbers[1];
    let x_130 : f32 = color.x;
    color.x = (x_130 + f32(x_127));
  }
  let x_134 : f32 = uv.x;
  if ((x_134 > 0.5)) {
    let x_139 : i32 = obj.numbers[2];
    let x_142 : f32 = color.y;
    color.y = (x_142 + f32(x_139));
  }
  let x_146 : f32 = uv.x;
  if ((x_146 > 0.75)) {
    let x_151 : i32 = obj.numbers[3];
    let x_154 : f32 = color.z;
    color.z = (x_154 + f32(x_151));
  }
  let x_158 : i32 = obj.numbers[4];
  let x_161 : f32 = color.y;
  color.y = (x_161 + f32(x_158));
  let x_165 : f32 = uv.y;
  if ((x_165 > 0.25)) {
    let x_170 : i32 = obj.numbers[5];
    let x_173 : f32 = color.x;
    color.x = (x_173 + f32(x_170));
  }
  let x_177 : f32 = uv.y;
  if ((x_177 > 0.5)) {
    let x_182 : i32 = obj.numbers[6];
    let x_185 : f32 = color.y;
    color.y = (x_185 + f32(x_182));
  }
  let x_189 : f32 = uv.y;
  if ((x_189 > 0.75)) {
    let x_194 : i32 = obj.numbers[7];
    let x_197 : f32 = color.z;
    color.z = (x_197 + f32(x_194));
  }
  let x_201 : i32 = obj.numbers[8];
  let x_204 : f32 = color.z;
  color.z = (x_204 + f32(x_201));
  let x_208 : f32 = uv.x;
  let x_210 : f32 = uv.y;
  if ((abs((x_208 - x_210)) < 0.25)) {
    let x_217 : i32 = obj.numbers[9];
    let x_220 : f32 = color.x;
    color.x = (x_220 + f32(x_217));
  }
  let x_223 : vec3<f32> = color;
  let x_224 : vec3<f32> = normalize(x_223);
  x_GLF_color = vec4<f32>(x_224.x, x_224.y, x_224.z, 1.0);
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
