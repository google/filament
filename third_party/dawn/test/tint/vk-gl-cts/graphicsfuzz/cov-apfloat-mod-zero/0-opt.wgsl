struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var undefined : f32;
  var x_51 : bool;
  var x_52_phi : bool;
  undefined = (5.0 - (0.0 * floor((5.0 / 5.0f))));
  let x_10 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_11 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_12 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_44 : bool = (x_10 == (x_11 + x_12));
  x_52_phi = x_44;
  if (!(x_44)) {
    let x_48 : f32 = undefined;
    let x_50 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    x_51 = (x_48 > x_50);
    x_52_phi = x_51;
  }
  let x_52 : bool = x_52_phi;
  if (x_52) {
    let x_15 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_16 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_17 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_18 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_15), f32(x_16), f32(x_17), f32(x_18));
  } else {
    let x_19 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_66 : f32 = f32(x_19);
    x_GLF_color = vec4<f32>(x_66, x_66, x_66, x_66);
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
