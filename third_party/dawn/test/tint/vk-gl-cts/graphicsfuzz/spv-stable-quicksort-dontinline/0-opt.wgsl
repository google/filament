struct QuicksortObject {
  numbers : array<i32, 10u>,
}

struct buf0 {
  resolution : vec2<f32>,
}

var<private> obj : QuicksortObject;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_32 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn swap_i1_i1_(i : ptr<function, i32>, j : ptr<function, i32>) {
  var temp : i32;
  let x_225 : i32 = *(i);
  let x_227 : i32 = obj.numbers[x_225];
  temp = x_227;
  let x_228 : i32 = *(i);
  let x_229 : i32 = *(j);
  let x_231 : i32 = obj.numbers[x_229];
  obj.numbers[x_228] = x_231;
  let x_233 : i32 = *(j);
  let x_234 : i32 = temp;
  obj.numbers[x_233] = x_234;
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
  let x_237 : i32 = *(h);
  let x_239 : i32 = obj.numbers[x_237];
  pivot = x_239;
  let x_240 : i32 = *(l);
  i_1 = (x_240 - 1);
  let x_242 : i32 = *(l);
  j_1 = x_242;
  loop {
    let x_247 : i32 = j_1;
    let x_248 : i32 = *(h);
    if ((x_247 <= (x_248 - 1))) {
    } else {
      break;
    }
    let x_252 : i32 = j_1;
    let x_254 : i32 = obj.numbers[x_252];
    let x_255 : i32 = pivot;
    if ((x_254 <= x_255)) {
      let x_259 : i32 = i_1;
      i_1 = (x_259 + 1);
      let x_261 : i32 = i_1;
      param = x_261;
      let x_262 : i32 = j_1;
      param_1 = x_262;
      swap_i1_i1_(&(param), &(param_1));
    }

    continuing {
      let x_264 : i32 = j_1;
      j_1 = (x_264 + 1);
    }
  }
  let x_266 : i32 = i_1;
  i_1 = (x_266 + 1);
  let x_268 : i32 = i_1;
  param_2 = x_268;
  let x_269 : i32 = *(h);
  param_3 = x_269;
  swap_i1_i1_(&(param_2), &(param_3));
  let x_271 : i32 = i_1;
  return x_271;
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
  let x_273 : i32 = top;
  let x_274 : i32 = (x_273 + 1);
  top = x_274;
  let x_275 : i32 = l_1;
  stack[x_274] = x_275;
  let x_277 : i32 = top;
  let x_278 : i32 = (x_277 + 1);
  top = x_278;
  let x_279 : i32 = h_1;
  stack[x_278] = x_279;
  loop {
    let x_285 : i32 = top;
    if ((x_285 >= 0)) {
    } else {
      break;
    }
    let x_288 : i32 = top;
    top = (x_288 - 1);
    let x_291 : i32 = stack[x_288];
    h_1 = x_291;
    let x_292 : i32 = top;
    top = (x_292 - 1);
    let x_295 : i32 = stack[x_292];
    l_1 = x_295;
    let x_296 : i32 = l_1;
    param_4 = x_296;
    let x_297 : i32 = h_1;
    param_5 = x_297;
    let x_298 : i32 = performPartition_i1_i1_(&(param_4), &(param_5));
    p = x_298;
    let x_299 : i32 = p;
    let x_301 : i32 = l_1;
    if (((x_299 - 1) > x_301)) {
      let x_305 : i32 = top;
      let x_306 : i32 = (x_305 + 1);
      top = x_306;
      let x_307 : i32 = l_1;
      stack[x_306] = x_307;
      let x_309 : i32 = top;
      let x_310 : i32 = (x_309 + 1);
      top = x_310;
      let x_311 : i32 = p;
      stack[x_310] = (x_311 - 1);
    }
    let x_314 : i32 = p;
    let x_316 : i32 = h_1;
    if (((x_314 + 1) < x_316)) {
      let x_320 : i32 = top;
      let x_321 : i32 = (x_320 + 1);
      top = x_321;
      let x_322 : i32 = p;
      stack[x_321] = (x_322 + 1);
      let x_325 : i32 = top;
      let x_326 : i32 = (x_325 + 1);
      top = x_326;
      let x_327 : i32 = h_1;
      stack[x_326] = x_327;
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
    let x_85 : i32 = i_2;
    if ((x_85 < 10)) {
    } else {
      break;
    }
    let x_88 : i32 = i_2;
    let x_89 : i32 = i_2;
    obj.numbers[x_88] = (10 - x_89);
    let x_92 : i32 = i_2;
    let x_93 : i32 = i_2;
    let x_95 : i32 = obj.numbers[x_93];
    let x_96 : i32 = i_2;
    let x_98 : i32 = obj.numbers[x_96];
    obj.numbers[x_92] = (x_95 * x_98);

    continuing {
      let x_101 : i32 = i_2;
      i_2 = (x_101 + 1);
    }
  }
  quicksort_();
  let x_104 : vec4<f32> = gl_FragCoord;
  let x_107 : vec2<f32> = x_32.resolution;
  uv = (vec2<f32>(x_104.x, x_104.y) / x_107);
  color = vec3<f32>(1.0, 2.0, 3.0);
  let x_110 : i32 = obj.numbers[0];
  let x_113 : f32 = color.x;
  color.x = (x_113 + f32(x_110));
  let x_117 : f32 = uv.x;
  if ((x_117 > 0.25)) {
    let x_122 : i32 = obj.numbers[1];
    let x_125 : f32 = color.x;
    color.x = (x_125 + f32(x_122));
  }
  let x_129 : f32 = uv.x;
  if ((x_129 > 0.5)) {
    let x_134 : i32 = obj.numbers[2];
    let x_137 : f32 = color.y;
    color.y = (x_137 + f32(x_134));
  }
  let x_141 : f32 = uv.x;
  if ((x_141 > 0.75)) {
    let x_146 : i32 = obj.numbers[3];
    let x_149 : f32 = color.z;
    color.z = (x_149 + f32(x_146));
  }
  let x_153 : i32 = obj.numbers[4];
  let x_156 : f32 = color.y;
  color.y = (x_156 + f32(x_153));
  let x_160 : f32 = uv.y;
  if ((x_160 > 0.25)) {
    let x_165 : i32 = obj.numbers[5];
    let x_168 : f32 = color.x;
    color.x = (x_168 + f32(x_165));
  }
  let x_172 : f32 = uv.y;
  if ((x_172 > 0.5)) {
    let x_177 : i32 = obj.numbers[6];
    let x_180 : f32 = color.y;
    color.y = (x_180 + f32(x_177));
  }
  let x_184 : f32 = uv.y;
  if ((x_184 > 0.75)) {
    let x_189 : i32 = obj.numbers[7];
    let x_192 : f32 = color.z;
    color.z = (x_192 + f32(x_189));
  }
  let x_196 : i32 = obj.numbers[8];
  let x_199 : f32 = color.z;
  color.z = (x_199 + f32(x_196));
  let x_203 : f32 = uv.x;
  let x_205 : f32 = uv.y;
  if ((abs((x_203 - x_205)) < 0.25)) {
    let x_212 : i32 = obj.numbers[9];
    let x_215 : f32 = color.x;
    color.x = (x_215 + f32(x_212));
  }
  let x_218 : vec3<f32> = color;
  let x_219 : vec3<f32> = normalize(x_218);
  x_GLF_color = vec4<f32>(x_219.x, x_219.y, x_219.z, 1.0);
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
