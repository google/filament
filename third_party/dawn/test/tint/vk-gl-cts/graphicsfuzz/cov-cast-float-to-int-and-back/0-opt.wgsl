struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn main_1() {
  var f : f32;
  f = 142.699996948;
  let x_25 : f32 = f;
  if ((f32(i32(x_25)) > 100.0)) {
    let x_33 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_36 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_39 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_42 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_33), f32(x_36), f32(x_39), f32(x_42));
  } else {
    let x_46 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_47 : f32 = f32(x_46);
    x_GLF_color = vec4<f32>(x_47, x_47, x_47, x_47);
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
