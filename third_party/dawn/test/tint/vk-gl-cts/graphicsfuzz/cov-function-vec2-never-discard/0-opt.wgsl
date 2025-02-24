struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct buf2 {
  zero : f32,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_8 : buf1;

@group(0) @binding(2) var<uniform> x_10 : buf2;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_13 : buf0;

fn func_vf2_(pos : ptr<function, vec2<f32>>) -> bool {
  let x_62 : f32 = (*(pos)).x;
  let x_64 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_62 < x_64)) {
    return true;
  }
  let x_69 : f32 = (*(pos)).y;
  let x_71 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_69 > x_71)) {
    return false;
  }
  let x_76 : f32 = x_10.zero;
  let x_78 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_76 > x_78)) {
    return true;
  }
  return true;
}

fn main_1() {
  var param : vec2<f32>;
  let x_42 : vec4<f32> = gl_FragCoord;
  param = vec2<f32>(x_42.x, x_42.y);
  let x_44 : bool = func_vf2_(&(param));
  if (x_44) {
    discard;
  }
  let x_48 : i32 = x_13.x_GLF_uniform_int_values[0].el;
  let x_51 : i32 = x_13.x_GLF_uniform_int_values[1].el;
  let x_54 : i32 = x_13.x_GLF_uniform_int_values[1].el;
  let x_57 : i32 = x_13.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4<f32>(f32(x_48), f32(x_51), f32(x_54), f32(x_57));
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
