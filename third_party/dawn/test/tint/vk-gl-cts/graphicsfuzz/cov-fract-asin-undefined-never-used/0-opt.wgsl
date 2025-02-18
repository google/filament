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

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_10 : buf1;

fn main_1() {
  var f0 : f32;
  var f1 : f32;
  f0 = 1.0;
  let x_35 : f32 = f0;
  f1 = fract(x_35);
  let x_38 : f32 = gl_FragCoord.x;
  let x_40 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_38 > x_40)) {
    let x_46 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_49 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_52 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_55 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_46), f32(x_49), f32(x_52), f32(x_55));
  } else {
    let x_58 : f32 = f1;
    x_GLF_color = vec4<f32>(x_58, x_58, x_58, x_58);
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
