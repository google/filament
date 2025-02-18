struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_7 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_9 : buf0;

fn main_1() {
  var a : f32;
  var b : f32;
  a = 1.0;
  let x_33 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  let x_34 : f32 = a;
  b = clamp(x_33, select(0.0, x_34, false), 1.0);
  let x_37 : f32 = b;
  let x_39 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  if ((x_37 == x_39)) {
    let x_44 : f32 = b;
    let x_46 : f32 = x_7.x_GLF_uniform_float_values[0].el;
    let x_49 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_52 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_55 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>((x_44 * x_46), f32(x_49), f32(x_52), f32(x_55));
  } else {
    let x_59 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_60 : f32 = f32(x_59);
    x_GLF_color = vec4<f32>(x_60, x_60, x_60, x_60);
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
