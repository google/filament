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

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_8 : buf1;

fn main_1() {
  var v : vec4<f32>;
  let x_33 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  v = clamp(cosh(vec4<f32>(1.0, 1.0, 1.0, 1.0)), vec4<f32>(x_33, x_33, x_33, x_33), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_38 : f32 = v.x;
  let x_40 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  let x_43 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  let x_46 : f32 = v.z;
  x_GLF_color = vec4<f32>(x_38, f32(x_40), f32(x_43), x_46);
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
