struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_14 : buf1;

fn f1_f1_(a : ptr<function, f32>) -> f32 {
  var b : i32;
  var c : f32;
  b = 8;
  let x_71 : f32 = gl_FragCoord.y;
  let x_73 : f32 = x_9.x_GLF_uniform_float_values[0].el;
  if ((x_71 >= x_73)) {
    let x_77 : i32 = b;
    b = (x_77 + 1);
    let x_79 : i32 = b;
    b = (x_79 + 1);
  }
  let x_81 : f32 = *(a);
  let x_83 : f32 = x_9.x_GLF_uniform_float_values[1].el;
  if ((x_81 < x_83)) {
    let x_88 : f32 = x_9.x_GLF_uniform_float_values[1].el;
    return x_88;
  }
  let x_89 : i32 = b;
  c = f32(clamp(x_89, 0, 2));
  let x_92 : f32 = c;
  return x_92;
}

fn main_1() {
  var a_1 : f32;
  var param : f32;
  let x_43 : f32 = x_9.x_GLF_uniform_float_values[1].el;
  param = x_43;
  let x_44 : f32 = f1_f1_(&(param));
  a_1 = x_44;
  let x_45 : f32 = a_1;
  let x_47 : f32 = x_9.x_GLF_uniform_float_values[2].el;
  if ((x_45 == x_47)) {
    let x_53 : i32 = x_14.x_GLF_uniform_int_values[1].el;
    let x_56 : i32 = x_14.x_GLF_uniform_int_values[0].el;
    let x_59 : i32 = x_14.x_GLF_uniform_int_values[0].el;
    let x_62 : i32 = x_14.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_53), f32(x_56), f32(x_59), f32(x_62));
  } else {
    let x_66 : i32 = x_14.x_GLF_uniform_int_values[0].el;
    let x_67 : f32 = f32(x_66);
    x_GLF_color = vec4<f32>(x_67, x_67, x_67, x_67);
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
