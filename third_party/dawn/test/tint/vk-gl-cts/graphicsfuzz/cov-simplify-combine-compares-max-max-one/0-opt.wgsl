struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  let x_24 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_26 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  a = max(x_24, max(x_26, 1));
  let x_29 : i32 = a;
  let x_31 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_29 == x_31)) {
    let x_36 : i32 = a;
    let x_39 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_42 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_44 : i32 = a;
    x_GLF_color = vec4<f32>(f32(x_36), f32(x_39), f32(x_42), f32(x_44));
  } else {
    let x_47 : i32 = a;
    let x_48 : f32 = f32(x_47);
    x_GLF_color = vec4<f32>(x_48, x_48, x_48, x_48);
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
