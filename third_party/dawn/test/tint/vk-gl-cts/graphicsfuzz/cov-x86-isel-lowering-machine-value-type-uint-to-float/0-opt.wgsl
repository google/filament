struct strided_arr {
  @size(16)
  el : u32,
}

alias Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_uint_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf2 {
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

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(2) var<uniform> x_8 : buf2;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_12 : buf0;

fn main_1() {
  var a : u32;
  var b : f32;
  var c : u32;
  let x_38 : u32 = x_6.x_GLF_uniform_uint_values[0].el;
  let x_40 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  a = (x_38 >> u32(x_40));
  let x_43 : u32 = a;
  b = bitcast<f32>(x_43);
  let x_45 : f32 = b;
  c = bitcast<u32>(x_45);
  let x_47 : u32 = c;
  let x_49 : u32 = x_6.x_GLF_uniform_uint_values[0].el;
  if ((x_47 == x_49)) {
    let x_55 : i32 = x_12.x_GLF_uniform_int_values[0].el;
    let x_58 : i32 = x_12.x_GLF_uniform_int_values[1].el;
    let x_61 : i32 = x_12.x_GLF_uniform_int_values[1].el;
    let x_64 : i32 = x_12.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_55), f32(x_58), f32(x_61), f32(x_64));
  } else {
    let x_67 : f32 = b;
    x_GLF_color = vec4<f32>(x_67, x_67, x_67, x_67);
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
