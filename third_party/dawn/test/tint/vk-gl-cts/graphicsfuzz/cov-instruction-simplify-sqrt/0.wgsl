struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

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

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_7 : buf1;

fn main_1() {
  let x_31 : f32 = x_5.x_GLF_uniform_float_values[0].el;
  if ((sqrt(x_31) < -1.0)) {
    let x_10 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_38 : f32 = f32(x_10);
    x_GLF_color = vec4<f32>(x_38, x_38, x_38, x_38);
  } else {
    let x_11 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_41 : f32 = f32(x_11);
    let x_12 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_43 : f32 = f32(x_12);
    x_GLF_color = vec4<f32>(x_41, x_43, x_43, x_41);
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
