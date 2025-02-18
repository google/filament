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

fn swap_i1_i1_(i : ptr<function, i32>, j : ptr<function, i32>) {
  var temp : i32;
  let x_239 : i32 = *(i);
  let x_241 : i32 = obj.numbers[x_239];
  temp = x_241;
  let x_242 : i32 = *(i);
  let x_243 : i32 = *(j);
  let x_245 : i32 = obj.numbers[x_243];
  obj.numbers[x_242] = x_245;
  let x_247 : i32 = *(j);
  let x_248 : i32 = temp;
  obj.numbers[x_247] = x_248;
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
  let x_251 : i32 = *(h);
  let x_253 : i32 = obj.numbers[x_251];
  pivot = x_253;
  let x_254 : i32 = *(l);
  i_1 = (x_254 - 1);
  let x_256 : i32 = *(l);
  j_1 = x_256;
  loop {
    let x_261 : i32 = j_1;
    let x_262 : i32 = *(h);
    if ((x_261 <= (x_262 - 1))) {
    } else {
      break;
    }
    let x_266 : i32 = j_1;
    let x_268 : i32 = obj.numbers[x_266];
    let x_269 : i32 = pivot;
    if ((x_268 <= x_269)) {
      let x_273 : i32 = i_1;
      i_1 = (x_273 + 1);
      let x_275 : i32 = i_1;
      param = x_275;
      let x_276 : i32 = j_1;
      param_1 = x_276;
      swap_i1_i1_(&(param), &(param_1));
    }

    continuing {
      let x_278 : i32 = j_1;
      j_1 = (x_278 + 1);
    }
  }
  let x_280 : i32 = i_1;
  param_2 = (x_280 + 1);
  let x_282 : i32 = *(h);
  param_3 = x_282;
  swap_i1_i1_(&(param_2), &(param_3));
  let x_284 : i32 = i_1;
  return (x_284 + 1);
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
  let x_287 : i32 = top;
  let x_288 : i32 = (x_287 + 1);
  top = x_288;
  let x_289 : i32 = l_1;
  stack[x_288] = x_289;
  let x_291 : i32 = top;
  let x_292 : i32 = (x_291 + 1);
  top = x_292;
  let x_293 : i32 = h_1;
  stack[x_292] = x_293;
  loop {
    let x_299 : i32 = top;
    if ((x_299 >= 0)) {
    } else {
      break;
    }
    let x_302 : i32 = top;
    top = (x_302 - 1);
    let x_305 : i32 = stack[x_302];
    h_1 = x_305;
    let x_306 : i32 = top;
    top = (x_306 - 1);
    let x_309 : i32 = stack[x_306];
    l_1 = x_309;
    let x_310 : i32 = l_1;
    param_4 = x_310;
    let x_311 : i32 = h_1;
    param_5 = x_311;
    let x_312 : i32 = performPartition_i1_i1_(&(param_4), &(param_5));
    p = x_312;
    let x_313 : i32 = p;
    let x_315 : i32 = l_1;
    if (((x_313 - 1) > x_315)) {
      let x_319 : i32 = top;
      let x_320 : i32 = (x_319 + 1);
      top = x_320;
      let x_321 : i32 = l_1;
      stack[x_320] = x_321;
      let x_323 : i32 = top;
      let x_324 : i32 = (x_323 + 1);
      top = x_324;
      let x_325 : i32 = p;
      stack[x_324] = (x_325 - 1);
    }
    let x_328 : i32 = p;
    let x_330 : i32 = h_1;
    if (((x_328 + 1) < x_330)) {
      let x_334 : i32 = top;
      let x_335 : i32 = (x_334 + 1);
      top = x_335;
      let x_336 : i32 = p;
      stack[x_335] = (x_336 + 1);
      let x_339 : i32 = top;
      let x_340 : i32 = (x_339 + 1);
      top = x_340;
      let x_341 : i32 = h_1;
      stack[x_340] = x_341;
    }
  }
  return;
}

fn main_1() {
  var i_2 : i32;
  var uv : vec2<f32>;
  var color : vec3<f32>;
  let x_90 : vec4<f32> = x_GLF_pos;
  x_GLF_FragCoord = ((x_90 + vec4<f32>(1.0, 1.0, 0.0, 0.0)) * vec4<f32>(128.0, 128.0, 1.0, 1.0));
  i_2 = 0;
  loop {
    let x_97 : i32 = i_2;
    if ((x_97 < 10)) {
    } else {
      break;
    }
    let x_100 : i32 = i_2;
    let x_101 : i32 = i_2;
    obj.numbers[x_100] = (10 - x_101);
    let x_104 : i32 = i_2;
    let x_105 : i32 = i_2;
    let x_107 : i32 = obj.numbers[x_105];
    let x_108 : i32 = i_2;
    let x_110 : i32 = obj.numbers[x_108];
    obj.numbers[x_104] = (x_107 * x_110);

    continuing {
      let x_113 : i32 = i_2;
      i_2 = (x_113 + 1);
    }
  }
  quicksort_();
  let x_116 : vec4<f32> = x_GLF_FragCoord;
  let x_119 : vec2<f32> = x_34.resolution;
  uv = (vec2<f32>(x_116.x, x_116.y) / x_119);
  color = vec3<f32>(1.0, 2.0, 3.0);
  let x_122 : i32 = obj.numbers[0];
  let x_125 : f32 = color.x;
  color.x = (x_125 + f32(x_122));
  let x_129 : f32 = uv.x;
  if ((x_129 > 0.25)) {
    let x_134 : i32 = obj.numbers[1];
    let x_137 : f32 = color.x;
    color.x = (x_137 + f32(x_134));
  }
  let x_141 : f32 = uv.x;
  if ((x_141 > 0.5)) {
    let x_146 : i32 = obj.numbers[2];
    let x_149 : f32 = color.y;
    color.y = (x_149 + f32(x_146));
  }
  let x_153 : f32 = uv.x;
  if ((x_153 > 0.75)) {
    let x_158 : i32 = obj.numbers[3];
    let x_161 : f32 = color.z;
    color.z = (x_161 + f32(x_158));
  }
  let x_165 : i32 = obj.numbers[4];
  let x_168 : f32 = color.y;
  color.y = (x_168 + f32(x_165));
  let x_172 : f32 = uv.y;
  if ((x_172 > 0.25)) {
    let x_177 : i32 = obj.numbers[5];
    let x_180 : f32 = color.x;
    color.x = (x_180 + f32(x_177));
  }
  let x_184 : f32 = uv.y;
  if ((x_184 > 0.5)) {
    let x_189 : i32 = obj.numbers[6];
    let x_192 : f32 = color.y;
    color.y = (x_192 + f32(x_189));
  }
  let x_196 : f32 = uv.y;
  if ((x_196 > 0.75)) {
    let x_201 : i32 = obj.numbers[7];
    let x_204 : f32 = color.z;
    color.z = (x_204 + f32(x_201));
  }
  let x_208 : i32 = obj.numbers[8];
  let x_211 : f32 = color.z;
  color.z = (x_211 + f32(x_208));
  let x_215 : f32 = uv.x;
  let x_217 : f32 = uv.y;
  if ((abs((x_215 - x_217)) < 0.25)) {
    let x_224 : i32 = obj.numbers[9];
    let x_227 : f32 = color.x;
    color.x = (x_227 + f32(x_224));
  }
  let x_230 : vec3<f32> = color;
  let x_231 : vec3<f32> = normalize(x_230);
  frag_color = vec4<f32>(x_231.x, x_231.y, x_231.z, 1.0);
  let x_236 : vec4<f32> = x_GLF_pos;
  gl_Position = x_236;
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
