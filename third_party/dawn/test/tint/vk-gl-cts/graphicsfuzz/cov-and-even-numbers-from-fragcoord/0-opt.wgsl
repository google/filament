struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 3u>;

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

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_6 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

fn main_1() {
  var v : vec2<i32>;
  let x_39 : f32 = gl_FragCoord.y;
  let x_41 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_39 < x_41)) {
    let x_47 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    let x_48 : f32 = f32(x_47);
    x_GLF_color = vec4<f32>(x_48, x_48, x_48, x_48);
  } else {
    let x_50 : vec4<f32> = gl_FragCoord;
    let x_53 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_55 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_59 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    v = vec2<i32>(((vec2<f32>(x_50.x, x_50.y) - vec2<f32>(x_53, x_55)) * x_59));
    let x_63 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_65 : i32 = v.y;
    let x_67 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    let x_70 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    let x_74 : i32 = v.x;
    let x_76 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    let x_80 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    x_GLF_color = vec4<f32>(x_63, f32(((x_65 - x_67) & x_70)), f32((x_74 & x_76)), x_80);
  }
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
