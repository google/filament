struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf2 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

struct strided_arr_2 {
  @size(16)
  el : u32,
}

alias Arr_2 = array<strided_arr_2, 1u>;

struct buf1 {
  x_GLF_uniform_uint_values : Arr_2,
}

@group(0) @binding(2) var<uniform> x_8 : buf2;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_10 : buf0;

@group(0) @binding(1) var<uniform> x_12 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : u32;
  var b : i32;
  a = 0u;
  let x_41 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  b = x_41;
  let x_43 : f32 = gl_FragCoord.x;
  let x_45 : f32 = x_10.x_GLF_uniform_float_values[0].el;
  if ((x_43 < x_45)) {
    let x_50 : u32 = x_12.x_GLF_uniform_uint_values[0].el;
    let x_51 : u32 = a;
    b = bitcast<i32>((x_50 % x_51));
  }
  let x_54 : i32 = b;
  let x_56 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  if ((x_54 == x_56)) {
    let x_62 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    let x_65 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    let x_68 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    let x_71 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_62), f32(x_65), f32(x_68), f32(x_71));
  } else {
    let x_75 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    let x_76 : f32 = f32(x_75);
    x_GLF_color = vec4<f32>(x_76, x_76, x_76, x_76);
  }
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
