struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  let x_28 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_28;
  let x_29 : i32 = a;
  let x_31 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_37 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  if ((((vec2<i32>(x_29, x_29) / vec2<i32>(x_31, 63677))).y == x_37)) {
    let x_43 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_46 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_49 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_52 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_43), f32(x_46), f32(x_49), f32(x_52));
  } else {
    let x_56 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_57 : f32 = f32(x_56);
    x_GLF_color = vec4<f32>(x_57, x_57, x_57, x_57);
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
