struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_11 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var arr : array<i32, 3u>;
  var a : i32;
  var b : i32;
  var c : i32;
  let x_40 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_42 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_44 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  arr = array<i32, 3u>(x_40, x_42, x_44);
  let x_47 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  let x_49 : i32 = arr[x_47];
  a = x_49;
  let x_50 : i32 = a;
  b = (x_50 - 1);
  let x_53 : f32 = gl_FragCoord.x;
  let x_55 : f32 = x_11.x_GLF_uniform_float_values[0].el;
  if ((x_53 < x_55)) {
    let x_59 : i32 = b;
    b = (x_59 + 1);
  }
  let x_62 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  c = x_62;
  let x_63 : i32 = c;
  let x_65 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_67 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  let x_69 : i32 = b;
  arr[clamp(x_63, x_65, x_67)] = x_69;
  let x_72 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  let x_74 : i32 = arr[x_72];
  let x_77 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_79 : i32 = arr[x_77];
  let x_82 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_84 : i32 = arr[x_82];
  let x_87 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  let x_89 : i32 = arr[x_87];
  x_GLF_color = vec4<f32>(f32(x_74), f32(x_79), f32(x_84), f32(x_89));
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
