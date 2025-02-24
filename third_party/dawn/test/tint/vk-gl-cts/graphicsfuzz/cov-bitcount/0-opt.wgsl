struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

@group(0) @binding(1) var<uniform> x_11 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn f1_() -> i32 {
  var a : i32;
  var i : i32;
  a = 256;
  let x_65 : f32 = gl_FragCoord.y;
  let x_67 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_65 > x_67)) {
    let x_71 : i32 = a;
    a = (x_71 + 1);
  }
  let x_73 : i32 = a;
  i = countOneBits(x_73);
  let x_75 : i32 = i;
  let x_77 : i32 = x_11.x_GLF_uniform_int_values[0].el;
  if ((x_75 < x_77)) {
    let x_82 : i32 = x_11.x_GLF_uniform_int_values[0].el;
    return x_82;
  }
  let x_83 : i32 = i;
  return x_83;
}

fn main_1() {
  var a_1 : i32;
  let x_38 : i32 = f1_();
  a_1 = x_38;
  let x_39 : i32 = a_1;
  let x_41 : i32 = x_11.x_GLF_uniform_int_values[2].el;
  if ((x_39 == x_41)) {
    let x_47 : i32 = x_11.x_GLF_uniform_int_values[0].el;
    let x_50 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    let x_53 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    let x_56 : i32 = x_11.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_47), f32(x_50), f32(x_53), f32(x_56));
  } else {
    let x_60 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    let x_61 : f32 = f32(x_60);
    x_GLF_color = vec4<f32>(x_61, x_61, x_61, x_61);
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
