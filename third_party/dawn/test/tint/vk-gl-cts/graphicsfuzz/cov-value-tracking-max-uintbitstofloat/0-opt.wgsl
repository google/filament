struct strided_arr {
  @size(16)
  el : u32,
}

alias Arr = array<strided_arr, 1u>;

struct buf2 {
  x_GLF_uniform_uint_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr_1,
}

struct strided_arr_2 {
  @size(16)
  el : i32,
}

alias Arr_2 = array<strided_arr_2, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_2,
}

@group(0) @binding(2) var<uniform> x_6 : buf2;

@group(0) @binding(1) var<uniform> x_8 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_10 : buf0;

fn main_1() {
  var f : f32;
  let x_36 : u32 = x_6.x_GLF_uniform_uint_values[0].el;
  f = bitcast<f32>(max(100u, x_36));
  let x_39 : f32 = f;
  let x_41 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_39 == x_41)) {
    let x_47 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_50 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_53 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_56 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_47), f32(x_50), f32(x_53), f32(x_56));
  } else {
    let x_60 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_61 : f32 = f32(x_60);
    x_GLF_color = vec4<f32>(x_61, x_61, x_61, x_61);
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
