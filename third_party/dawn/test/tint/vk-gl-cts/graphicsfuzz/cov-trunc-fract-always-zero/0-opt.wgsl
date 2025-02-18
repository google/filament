struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_7 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_9 : buf0;

fn main_1() {
  var f : f32;
  let x_35 : f32 = gl_FragCoord.y;
  let x_37 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  f = fract(trunc(select(1.0, 0.100000001, (x_35 < x_37))));
  let x_42 : f32 = f;
  let x_44 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  if ((x_42 == x_44)) {
    let x_50 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_53 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_56 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_59 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_50), f32(x_53), f32(x_56), f32(x_59));
  } else {
    let x_63 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_64 : f32 = f32(x_63);
    x_GLF_color = vec4<f32>(x_64, x_64, x_64, x_64);
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
