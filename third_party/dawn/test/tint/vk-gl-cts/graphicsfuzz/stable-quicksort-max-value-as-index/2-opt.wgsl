struct QuicksortObject {
  numbers : array<i32, 10u>,
}

struct buf1 {
  resolution : vec2<f32>,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> obj : QuicksortObject;

var<private> x_GLF_FragCoord : vec4<f32>;

var<private> x_GLF_pos : vec4<f32>;

@group(0) @binding(1) var<uniform> x_34 : buf1;

@group(0) @binding(0) var<uniform> x_37 : buf0;

var<private> frag_color : vec4<f32>;

var<private> gl_Position : vec4<f32>;

fn swap_i1_i1_(i : ptr<function, i32>, j : ptr<function, i32>) {
  var temp : i32;
  let x_257 : i32 = *(i);
  let x_259 : i32 = obj.numbers[x_257];
  temp = x_259;
  let x_260 : i32 = *(i);
  let x_261 : i32 = *(j);
  let x_263 : i32 = obj.numbers[x_261];
  obj.numbers[x_260] = x_263;
  let x_265 : i32 = *(j);
  let x_266 : i32 = temp;
  obj.numbers[x_265] = x_266;
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
  let x_269 : i32 = *(h);
  let x_271 : i32 = obj.numbers[x_269];
  pivot = x_271;
  let x_272 : i32 = *(l);
  i_1 = (x_272 - 1);
  let x_274 : i32 = *(l);
  j_1 = x_274;
  loop {
    let x_279 : i32 = j_1;
    let x_280 : i32 = *(h);
    if ((x_279 <= (x_280 - 1))) {
    } else {
      break;
    }
    let x_284 : i32 = j_1;
    let x_286 : i32 = obj.numbers[x_284];
    let x_287 : i32 = pivot;
    if ((x_286 <= x_287)) {
      let x_291 : i32 = i_1;
      i_1 = (x_291 + 1);
      let x_293 : i32 = i_1;
      param = x_293;
      let x_294 : i32 = j_1;
      param_1 = x_294;
      swap_i1_i1_(&(param), &(param_1));
    }

    continuing {
      let x_296 : i32 = j_1;
      j_1 = (x_296 + 1);
    }
  }
  let x_298 : i32 = i_1;
  param_2 = (x_298 + 1);
  let x_300 : i32 = *(h);
  param_3 = x_300;
  swap_i1_i1_(&(param_2), &(param_3));
  let x_302 : i32 = i_1;
  return (x_302 + 1);
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
  let x_305 : i32 = top;
  let x_306 : i32 = (x_305 + 1);
  top = x_306;
  let x_307 : i32 = l_1;
  stack[x_306] = x_307;
  let x_309 : i32 = top;
  let x_310 : i32 = (x_309 + 1);
  top = x_310;
  let x_311 : i32 = h_1;
  stack[x_310] = x_311;
  loop {
    let x_317 : i32 = top;
    if ((x_317 >= 0)) {
    } else {
      break;
    }
    let x_320 : i32 = top;
    top = (x_320 - 1);
    let x_323 : i32 = stack[x_320];
    h_1 = x_323;
    let x_324 : i32 = top;
    top = (x_324 - 1);
    let x_327 : i32 = stack[x_324];
    l_1 = x_327;
    let x_328 : i32 = l_1;
    param_4 = x_328;
    let x_329 : i32 = h_1;
    param_5 = x_329;
    let x_330 : i32 = performPartition_i1_i1_(&(param_4), &(param_5));
    p = x_330;
    let x_331 : i32 = p;
    let x_333 : i32 = l_1;
    if (((x_331 - 1) > x_333)) {
      let x_337 : i32 = top;
      let x_338 : i32 = (x_337 + 1);
      top = x_338;
      let x_339 : i32 = l_1;
      stack[x_338] = x_339;
      let x_341 : i32 = top;
      let x_342 : i32 = (x_341 + 1);
      top = x_342;
      let x_343 : i32 = p;
      stack[x_342] = (x_343 - 1);
    }
    let x_346 : i32 = p;
    let x_348 : i32 = h_1;
    if (((x_346 + 1) < x_348)) {
      let x_352 : i32 = top;
      let x_353 : i32 = (x_352 + 1);
      top = x_353;
      let x_354 : i32 = p;
      stack[x_353] = (x_354 + 1);
      let x_357 : i32 = top;
      let x_358 : i32 = (x_357 + 1);
      top = x_358;
      let x_359 : i32 = h_1;
      stack[x_358] = x_359;
    }
  }
  return;
}

fn main_1() {
  var i_2 : i32;
  var uv : vec2<f32>;
  var color : vec3<f32>;
  let x_94 : vec4<f32> = x_GLF_pos;
  x_GLF_FragCoord = ((x_94 + vec4<f32>(1.0, 1.0, 0.0, 0.0)) * vec4<f32>(128.0, 128.0, 1.0, 1.0));
  i_2 = 0;
  loop {
    let x_101 : i32 = i_2;
    if ((x_101 < 10)) {
    } else {
      break;
    }
    let x_104 : i32 = i_2;
    let x_105 : i32 = i_2;
    obj.numbers[x_104] = (10 - x_105);
    let x_108 : i32 = i_2;
    let x_109 : i32 = i_2;
    let x_111 : i32 = obj.numbers[x_109];
    let x_112 : i32 = i_2;
    let x_114 : i32 = obj.numbers[x_112];
    obj.numbers[x_108] = (x_111 * x_114);

    continuing {
      let x_117 : i32 = i_2;
      i_2 = (x_117 + 1);
    }
  }
  quicksort_();
  let x_120 : vec4<f32> = x_GLF_FragCoord;
  let x_123 : vec2<f32> = x_34.resolution;
  uv = (vec2<f32>(x_120.x, x_120.y) / x_123);
  color = vec3<f32>(1.0, 2.0, 3.0);
  let x_126 : i32 = obj.numbers[0];
  let x_129 : f32 = color.x;
  color.x = (x_129 + f32(x_126));
  let x_133 : f32 = uv.x;
  if ((x_133 > 0.25)) {
    let x_138 : i32 = obj.numbers[1];
    let x_141 : f32 = color.x;
    color.x = (x_141 + f32(x_138));
  }
  let x_145 : f32 = uv.x;
  if ((x_145 > 0.5)) {
    let x_150 : f32 = x_37.injectionSwitch.y;
    let x_155 : i32 = obj.numbers[max((2 * i32(x_150)), 2)];
    let x_158 : f32 = x_37.injectionSwitch.y;
    let x_163 : i32 = obj.numbers[max((2 * i32(x_158)), 2)];
    let x_167 : f32 = color.y;
    color.y = (x_167 + max(f32(x_155), f32(x_163)));
  }
  let x_171 : f32 = uv.x;
  if ((x_171 > 0.75)) {
    let x_176 : i32 = obj.numbers[3];
    let x_179 : f32 = color.z;
    color.z = (x_179 + f32(x_176));
  }
  let x_183 : i32 = obj.numbers[4];
  let x_186 : f32 = color.y;
  color.y = (x_186 + f32(x_183));
  let x_190 : f32 = uv.y;
  if ((x_190 > 0.25)) {
    let x_195 : i32 = obj.numbers[5];
    let x_198 : f32 = color.x;
    color.x = (x_198 + f32(x_195));
  }
  let x_202 : f32 = uv.y;
  if ((x_202 > 0.5)) {
    let x_207 : i32 = obj.numbers[6];
    let x_210 : f32 = color.y;
    color.y = (x_210 + f32(x_207));
  }
  let x_214 : f32 = uv.y;
  if ((x_214 > 0.75)) {
    let x_219 : i32 = obj.numbers[7];
    let x_222 : f32 = color.z;
    color.z = (x_222 + f32(x_219));
  }
  let x_226 : i32 = obj.numbers[8];
  let x_229 : f32 = color.z;
  color.z = (x_229 + f32(x_226));
  let x_233 : f32 = uv.x;
  let x_235 : f32 = uv.y;
  if ((abs((x_233 - x_235)) < 0.25)) {
    let x_242 : i32 = obj.numbers[9];
    let x_245 : f32 = color.x;
    color.x = (x_245 + f32(x_242));
  }
  let x_248 : vec3<f32> = color;
  let x_249 : vec3<f32> = normalize(x_248);
  frag_color = vec4<f32>(x_249.x, x_249.y, x_249.z, 1.0);
  let x_254 : vec4<f32> = x_GLF_pos;
  gl_Position = x_254;
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
