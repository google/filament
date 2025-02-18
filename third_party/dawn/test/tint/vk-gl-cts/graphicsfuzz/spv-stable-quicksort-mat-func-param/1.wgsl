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

fn swap_i1_i1_(i : ptr<function, i32>, j : ptr<function, i32>, x_228 : mat3x3<f32>) {
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
      swap_i1_i1_(&(param), &(param_1), mat3x3<f32>(vec3<f32>(0.0, 0.0, 0.0), vec3<f32>(0.0, 0.0, 0.0), vec3<f32>(0.0, 0.0, 0.0)));
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
  swap_i1_i1_(&(param_2), &(param_3), mat3x3<f32>(vec3<f32>(0.0, 0.0, 0.0), vec3<f32>(0.0, 0.0, 0.0), vec3<f32>(0.0, 0.0, 0.0)));
  let x_276 : i32 = i_1;
  return x_276;
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
  let x_278 : i32 = top;
  let x_279 : i32 = (x_278 + 1);
  top = x_279;
  let x_280 : i32 = l_1;
  stack[x_279] = x_280;
  let x_282 : i32 = top;
  let x_283 : i32 = (x_282 + 1);
  top = x_283;
  let x_284 : i32 = h_1;
  stack[x_283] = x_284;
  loop {
    let x_290 : i32 = top;
    if ((x_290 >= 0)) {
    } else {
      break;
    }
    let x_293 : i32 = top;
    top = (x_293 - 1);
    let x_296 : i32 = stack[x_293];
    h_1 = x_296;
    let x_297 : i32 = top;
    top = (x_297 - 1);
    let x_300 : i32 = stack[x_297];
    l_1 = x_300;
    let x_301 : i32 = l_1;
    param_4 = x_301;
    let x_302 : i32 = h_1;
    param_5 = x_302;
    let x_303 : i32 = performPartition_i1_i1_(&(param_4), &(param_5));
    p = x_303;
    let x_304 : i32 = p;
    let x_306 : i32 = l_1;
    if (((x_304 - 1) > x_306)) {
      let x_310 : i32 = top;
      let x_311 : i32 = (x_310 + 1);
      top = x_311;
      let x_312 : i32 = l_1;
      stack[x_311] = x_312;
      let x_314 : i32 = top;
      let x_315 : i32 = (x_314 + 1);
      top = x_315;
      let x_316 : i32 = p;
      stack[x_315] = (x_316 - 1);
    }
    let x_319 : i32 = p;
    let x_321 : i32 = h_1;
    if (((x_319 + 1) < x_321)) {
      let x_325 : i32 = top;
      let x_326 : i32 = (x_325 + 1);
      top = x_326;
      let x_327 : i32 = p;
      stack[x_326] = (x_327 + 1);
      let x_330 : i32 = top;
      let x_331 : i32 = (x_330 + 1);
      top = x_331;
      let x_332 : i32 = h_1;
      stack[x_331] = x_332;
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
    let x_89 : i32 = i_2;
    if ((x_89 < 10)) {
    } else {
      break;
    }
    let x_92 : i32 = i_2;
    let x_93 : i32 = i_2;
    obj.numbers[x_92] = (10 - x_93);
    let x_96 : i32 = i_2;
    let x_97 : i32 = i_2;
    let x_99 : i32 = obj.numbers[x_97];
    let x_100 : i32 = i_2;
    let x_102 : i32 = obj.numbers[x_100];
    obj.numbers[x_96] = (x_99 * x_102);

    continuing {
      let x_105 : i32 = i_2;
      i_2 = (x_105 + 1);
    }
  }
  quicksort_();
  let x_108 : vec4<f32> = gl_FragCoord;
  let x_111 : vec2<f32> = x_32.resolution;
  uv = (vec2<f32>(x_108.x, x_108.y) / x_111);
  color = vec3<f32>(1.0, 2.0, 3.0);
  let x_114 : i32 = obj.numbers[0];
  let x_117 : f32 = color.x;
  color.x = (x_117 + f32(x_114));
  let x_121 : f32 = uv.x;
  if ((x_121 > 0.25)) {
    let x_126 : i32 = obj.numbers[1];
    let x_129 : f32 = color.x;
    color.x = (x_129 + f32(x_126));
  }
  let x_133 : f32 = uv.x;
  if ((x_133 > 0.5)) {
    let x_138 : i32 = obj.numbers[2];
    let x_141 : f32 = color.y;
    color.y = (x_141 + f32(x_138));
  }
  let x_145 : f32 = uv.x;
  if ((x_145 > 0.75)) {
    let x_150 : i32 = obj.numbers[3];
    let x_153 : f32 = color.z;
    color.z = (x_153 + f32(x_150));
  }
  let x_157 : i32 = obj.numbers[4];
  let x_160 : f32 = color.y;
  color.y = (x_160 + f32(x_157));
  let x_164 : f32 = uv.y;
  if ((x_164 > 0.25)) {
    let x_169 : i32 = obj.numbers[5];
    let x_172 : f32 = color.x;
    color.x = (x_172 + f32(x_169));
  }
  let x_176 : f32 = uv.y;
  if ((x_176 > 0.5)) {
    let x_181 : i32 = obj.numbers[6];
    let x_184 : f32 = color.y;
    color.y = (x_184 + f32(x_181));
  }
  let x_188 : f32 = uv.y;
  if ((x_188 > 0.75)) {
    let x_193 : i32 = obj.numbers[7];
    let x_196 : f32 = color.z;
    color.z = (x_196 + f32(x_193));
  }
  let x_200 : i32 = obj.numbers[8];
  let x_203 : f32 = color.z;
  color.z = (x_203 + f32(x_200));
  let x_207 : f32 = uv.x;
  let x_209 : f32 = uv.y;
  if ((abs((x_207 - x_209)) < 0.25)) {
    let x_216 : i32 = obj.numbers[9];
    let x_219 : f32 = color.x;
    color.x = (x_219 + f32(x_216));
  }
  let x_222 : vec3<f32> = color;
  let x_223 : vec3<f32> = normalize(x_222);
  x_GLF_color = vec4<f32>(x_223.x, x_223.y, x_223.z, 1.0);
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
