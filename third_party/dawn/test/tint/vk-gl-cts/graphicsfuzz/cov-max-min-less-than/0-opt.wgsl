struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

fn main_1() {
  var a : i32;
  let x_30 : f32 = gl_FragCoord.x;
  a = max(1, min(1, i32(x_30)));
  let x_34 : i32 = a;
  if ((x_34 < 2)) {
    let x_40 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_43 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_46 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(1.0, f32(x_40), f32(x_43), f32(x_46));
  } else {
    let x_50 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_51 : f32 = f32(x_50);
    x_GLF_color = vec4<f32>(x_51, x_51, x_51, x_51);
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
