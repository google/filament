struct QuicksortObject {
  numbers : array<i32, 10u>,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

struct buf1 {
  resolution : vec2<f32>,
}

var<private> obj : QuicksortObject;

var<private> x_GLF_FragCoord : vec4<f32>;

var<private> x_GLF_pos : vec4<f32>;

@group(0) @binding(0) var<uniform> x_33 : buf0;

@group(0) @binding(1) var<uniform> x_36 : buf1;

var<private> frag_color : vec4<f32>;

var<private> gl_Position : vec4<f32>;

fn swap_i1_i1_(i : ptr<function, i32>, j : ptr<function, i32>) {
  var temp : i32;
  let x_250 : i32 = *(i);
  let x_252 : i32 = obj.numbers[x_250];
  temp = x_252;
  let x_253 : i32 = *(i);
  let x_254 : i32 = *(j);
  let x_256 : i32 = obj.numbers[x_254];
  obj.numbers[x_253] = x_256;
  let x_258 : i32 = *(j);
  let x_259 : i32 = temp;
  obj.numbers[x_258] = x_259;
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
  let x_262 : i32 = *(h);
  let x_264 : i32 = obj.numbers[x_262];
  pivot = x_264;
  let x_265 : i32 = *(l);
  i_1 = (x_265 - 1);
  let x_267 : i32 = *(l);
  j_1 = x_267;
  loop {
    let x_272 : i32 = j_1;
    let x_273 : i32 = *(h);
    if ((x_272 <= (x_273 - 1))) {
    } else {
      break;
    }
    let x_277 : i32 = j_1;
    let x_279 : i32 = obj.numbers[x_277];
    let x_280 : i32 = pivot;
    if ((x_279 <= x_280)) {
      let x_284 : i32 = i_1;
      i_1 = (x_284 + 1);
      let x_286 : i32 = i_1;
      param = x_286;
      let x_287 : i32 = j_1;
      param_1 = x_287;
      swap_i1_i1_(&(param), &(param_1));
    }

    continuing {
      let x_289 : i32 = j_1;
      j_1 = (x_289 + 1);
    }
  }
  let x_291 : i32 = i_1;
  param_2 = (x_291 + 1);
  let x_293 : i32 = *(h);
  param_3 = x_293;
  swap_i1_i1_(&(param_2), &(param_3));
  let x_295 : i32 = i_1;
  return (x_295 + 1);
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
  let x_298 : i32 = top;
  let x_299 : i32 = (x_298 + 1);
  top = x_299;
  let x_300 : i32 = l_1;
  stack[x_299] = x_300;
  let x_302 : i32 = top;
  let x_303 : i32 = (x_302 + 1);
  top = x_303;
  let x_304 : i32 = h_1;
  stack[x_303] = x_304;
  loop {
    let x_310 : i32 = top;
    if ((x_310 >= 0)) {
    } else {
      break;
    }
    let x_313 : i32 = top;
    top = (x_313 - 1);
    let x_316 : i32 = stack[x_313];
    h_1 = x_316;
    let x_317 : i32 = top;
    top = (x_317 - 1);
    let x_320 : i32 = stack[x_317];
    l_1 = x_320;
    let x_321 : i32 = l_1;
    param_4 = x_321;
    let x_322 : i32 = h_1;
    param_5 = x_322;
    let x_323 : i32 = performPartition_i1_i1_(&(param_4), &(param_5));
    p = x_323;
    let x_324 : i32 = p;
    let x_326 : i32 = l_1;
    if (((x_324 - 1) > x_326)) {
      let x_330 : i32 = top;
      let x_331 : i32 = (x_330 + 1);
      top = x_331;
      let x_332 : i32 = l_1;
      stack[x_331] = x_332;
      let x_334 : i32 = top;
      let x_335 : i32 = (x_334 + 1);
      top = x_335;
      let x_336 : i32 = p;
      stack[x_335] = (x_336 - 1);
    }
    let x_339 : i32 = p;
    let x_341 : i32 = h_1;
    if (((x_339 + 1) < x_341)) {
      let x_345 : i32 = top;
      let x_346 : i32 = (x_345 + 1);
      top = x_346;
      let x_347 : i32 = p;
      stack[x_346] = (x_347 + 1);
      let x_350 : i32 = top;
      let x_351 : i32 = (x_350 + 1);
      top = x_351;
      let x_352 : i32 = h_1;
      stack[x_351] = x_352;
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
    let x_109 : f32 = x_33.injectionSwitch.x;
    let x_111 : f32 = x_33.injectionSwitch.y;
    if ((x_109 > x_111)) {
      break;
    }
    let x_115 : i32 = i_2;
    let x_116 : i32 = i_2;
    let x_118 : i32 = obj.numbers[x_116];
    let x_119 : i32 = i_2;
    let x_121 : i32 = obj.numbers[x_119];
    obj.numbers[x_115] = (x_118 * x_121);

    continuing {
      let x_124 : i32 = i_2;
      i_2 = (x_124 + 1);
    }
  }
  quicksort_();
  let x_127 : vec4<f32> = x_GLF_FragCoord;
  let x_130 : vec2<f32> = x_36.resolution;
  uv = (vec2<f32>(x_127.x, x_127.y) / x_130);
  color = vec3<f32>(1.0, 2.0, 3.0);
  let x_133 : i32 = obj.numbers[0];
  let x_136 : f32 = color.x;
  color.x = (x_136 + f32(x_133));
  let x_140 : f32 = uv.x;
  if ((x_140 > 0.25)) {
    let x_145 : i32 = obj.numbers[1];
    let x_148 : f32 = color.x;
    color.x = (x_148 + f32(x_145));
  }
  let x_152 : f32 = uv.x;
  if ((x_152 > 0.5)) {
    let x_157 : i32 = obj.numbers[2];
    let x_160 : f32 = color.y;
    color.y = (x_160 + f32(x_157));
  }
  let x_164 : f32 = uv.x;
  if ((x_164 > 0.75)) {
    let x_169 : i32 = obj.numbers[3];
    let x_172 : f32 = color.z;
    color.z = (x_172 + f32(x_169));
  }
  let x_176 : i32 = obj.numbers[4];
  let x_179 : f32 = color.y;
  color.y = (x_179 + f32(x_176));
  let x_183 : f32 = uv.y;
  if ((x_183 > 0.25)) {
    let x_188 : i32 = obj.numbers[5];
    let x_191 : f32 = color.x;
    color.x = (x_191 + f32(x_188));
  }
  let x_195 : f32 = uv.y;
  if ((x_195 > 0.5)) {
    let x_200 : i32 = obj.numbers[6];
    let x_203 : f32 = color.y;
    color.y = (x_203 + f32(x_200));
  }
  let x_207 : f32 = uv.y;
  if ((x_207 > 0.75)) {
    let x_212 : i32 = obj.numbers[7];
    let x_215 : f32 = color.z;
    color.z = (x_215 + f32(x_212));
  }
  let x_219 : i32 = obj.numbers[8];
  let x_222 : f32 = color.z;
  color.z = (x_222 + f32(x_219));
  let x_226 : f32 = uv.x;
  let x_228 : f32 = uv.y;
  if ((abs((x_226 - x_228)) < 0.25)) {
    let x_235 : i32 = obj.numbers[9];
    let x_238 : f32 = color.x;
    color.x = (x_238 + f32(x_235));
  }
  let x_241 : vec3<f32> = color;
  let x_242 : vec3<f32> = normalize(x_241);
  frag_color = vec4<f32>(x_242.x, x_242.y, x_242.z, 1.0);
  let x_247 : vec4<f32> = x_GLF_pos;
  gl_Position = x_247;
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
