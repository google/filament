struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

fn main_1() {
  var a : f32;
  var b : f32;
  var c : f32;
  a = -1.0;
  b = 1.700000048;
  let x_27 : f32 = a;
  let x_28 : f32 = b;
  c = pow(x_27, x_28);
  let x_30 : f32 = c;
  x_GLF_color = vec4<f32>(x_30, x_30, x_30, x_30);
  let x_32 : f32 = a;
  let x_34 : f32 = b;
  if (((x_32 == -1.0) & (x_34 == 1.700000048))) {
    let x_41 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    let x_43 : f32 = x_8.x_GLF_uniform_float_values[1].el;
    let x_45 : f32 = x_8.x_GLF_uniform_float_values[1].el;
    let x_47 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4<f32>(x_41, x_43, x_45, x_47);
  } else {
    let x_50 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4<f32>(x_50, x_50, x_50, x_50);
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
