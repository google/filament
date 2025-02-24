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

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var numbers : array<i32, 3u>;
  var a : vec2<f32>;
  var b : f32;
  let x_38 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  numbers[x_38] = x_40;
  let x_43 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_45 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  numbers[x_43] = x_45;
  let x_48 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_50 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  numbers[x_48] = x_50;
  let x_53 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_56 : f32 = x_9.x_GLF_uniform_float_values[2].el;
  let x_60 : i32 = numbers[select(2, 1, (0.0 < x_56))];
  a = vec2<f32>(f32(x_53), f32(x_60));
  let x_63 : vec2<f32> = a;
  let x_65 : f32 = x_9.x_GLF_uniform_float_values[1].el;
  let x_67 : f32 = x_9.x_GLF_uniform_float_values[1].el;
  b = dot(x_63, vec2<f32>(x_65, x_67));
  let x_70 : f32 = b;
  let x_72 : f32 = x_9.x_GLF_uniform_float_values[0].el;
  if ((x_70 == x_72)) {
    let x_78 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_81 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_84 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_87 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_78), f32(x_81), f32(x_84), f32(x_87));
  } else {
    let x_91 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_92 : f32 = f32(x_91);
    x_GLF_color = vec4<f32>(x_92, x_92, x_92, x_92);
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
