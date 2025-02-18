struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr_1,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf0;

@group(0) @binding(1) var<uniform> x_8 : buf1;

fn main_1() {
  var a : f32;
  let x_10 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  let x_11 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_12 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_13 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4<f32>(f32(x_10), f32(x_11), f32(x_12), f32(x_13));
  let x_45 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  a = (1.0 - (x_45 * floor((1.0 / x_45))));
  let x_47 : f32 = a;
  let x_49 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_47 != x_49)) {
    let x_54 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    x_GLF_color.y = x_54;
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
