struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct buf2 {
  one : f32,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(2) var<uniform> x_11 : buf2;

@group(0) @binding(1) var<uniform> x_13 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f0 : f32;
  var f1 : f32;
  var f2 : f32;
  var f3 : f32;
  let x_36 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  f0 = x_36;
  let x_38 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_39 : f32 = f0;
  f1 = (x_38 * pow(x_39, 4.0));
  let x_43 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_44 : f32 = f0;
  f2 = (x_43 * pow(x_44, 4.0));
  let x_47 : f32 = f1;
  let x_48 : f32 = f2;
  let x_51 : f32 = x_11.one;
  let x_53 : f32 = f0;
  f3 = sqrt((((x_47 - x_48) - x_51) + x_53));
  let x_56 : f32 = f3;
  let x_59 : i32 = x_13.x_GLF_uniform_int_values[0].el;
  if ((i32(x_56) == x_59)) {
    let x_65 : i32 = x_13.x_GLF_uniform_int_values[0].el;
    let x_68 : i32 = x_13.x_GLF_uniform_int_values[1].el;
    let x_71 : i32 = x_13.x_GLF_uniform_int_values[1].el;
    let x_74 : i32 = x_13.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_65), f32(x_68), f32(x_71), f32(x_74));
  } else {
    let x_78 : i32 = x_13.x_GLF_uniform_int_values[1].el;
    let x_79 : f32 = f32(x_78);
    x_GLF_color = vec4<f32>(x_79, x_79, x_79, x_79);
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
