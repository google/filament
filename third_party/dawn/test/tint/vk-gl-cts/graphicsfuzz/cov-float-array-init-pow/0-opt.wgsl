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

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_7 : buf1;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var arr : array<f32, 10u>;
  f = 2.0;
  let x_37 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_39 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_41 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_43 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_45 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_47 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_49 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_50 : f32 = f;
  let x_52 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  let x_55 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  let x_57 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  arr = array<f32, 10u>(x_37, x_39, x_41, x_43, x_45, x_47, x_49, pow(x_50, x_52), x_55, x_57);
  let x_60 : i32 = x_9.x_GLF_uniform_int_values[0].el;
  let x_62 : f32 = arr[x_60];
  let x_65 : i32 = x_9.x_GLF_uniform_int_values[3].el;
  if ((i32(x_62) == x_65)) {
    let x_71 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_74 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    let x_77 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    let x_80 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_71), f32(x_74), f32(x_77), f32(x_80));
  } else {
    let x_84 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    let x_85 : f32 = f32(x_84);
    x_GLF_color = vec4<f32>(x_85, x_85, x_85, x_85);
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
