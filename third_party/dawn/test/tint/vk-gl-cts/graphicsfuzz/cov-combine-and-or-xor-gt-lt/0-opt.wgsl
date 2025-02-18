struct buf1 {
  v1 : vec2<f32>,
}

struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

fn main_1() {
  var b : bool;
  b = true;
  let x_38 : f32 = x_6.v1.x;
  let x_40 : f32 = x_6.v1.y;
  if ((x_38 > x_40)) {
    let x_45 : f32 = x_6.v1.x;
    let x_47 : f32 = x_6.v1.y;
    if ((x_45 < x_47)) {
      b = false;
    }
  }
  let x_51 : bool = b;
  if (x_51) {
    let x_10 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    let x_11 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    let x_12 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    let x_13 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_10), f32(x_11), f32(x_12), f32(x_13));
  } else {
    let x_14 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    let x_65 : f32 = f32(x_14);
    x_GLF_color = vec4<f32>(x_65, x_65, x_65, x_65);
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
