struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  let x_28 : f32 = gl_FragCoord.x;
  a = i32(x_28);
  let x_30 : i32 = a;
  if ((~(x_30) < 0)) {
    let x_36 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    a = x_36;
  }
  let x_37 : i32 = a;
  let x_39 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  if ((x_37 == x_39)) {
    let x_45 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_48 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_51 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_54 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_45), f32(x_48), f32(x_51), f32(x_54));
  } else {
    let x_58 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_59 : f32 = f32(x_58);
    x_GLF_color = vec4<f32>(x_59, x_59, x_59, x_59);
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
