struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var a : i32;
  f = 2.0;
  let x_27 : f32 = f;
  a = i32((1.0 - clamp(1.0, 1.0, x_27)));
  let x_31 : i32 = a;
  let x_33 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  if ((x_31 == x_33)) {
    let x_39 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_42 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_45 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_48 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_39), f32(x_42), f32(x_45), f32(x_48));
  } else {
    let x_52 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_53 : f32 = f32(x_52);
    x_GLF_color = vec4<f32>(x_53, x_53, x_53, x_53);
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
