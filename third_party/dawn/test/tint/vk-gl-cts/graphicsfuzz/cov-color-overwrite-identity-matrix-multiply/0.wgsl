struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 5u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  let x_33 : f32 = gl_FragCoord.x;
  let x_35 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_33 > x_35)) {
    let x_40 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    x_GLF_color = vec4<f32>(x_40, x_40, x_40, x_40);
    let x_43 : f32 = gl_FragCoord.y;
    if ((x_43 > x_35)) {
      let x_48 : f32 = x_6.x_GLF_uniform_float_values[4].el;
      x_GLF_color = vec4<f32>(x_48, x_48, x_48, x_48);
    }
    let x_51 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    x_GLF_color = vec4<f32>(x_51, x_51, x_51, x_51);
  }
  let x_54 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  x_GLF_color = vec4<f32>(x_35, x_54, x_54, 10.0);
  let x_61 : vec4<f32> = x_GLF_color;
  x_GLF_color = (mat4x4<f32>(vec4<f32>(x_35, 0.0, 0.0, 0.0), vec4<f32>(0.0, x_35, 0.0, 0.0), vec4<f32>(0.0, 0.0, x_35, 0.0), vec4<f32>(0.0, 0.0, 0.0, x_35)) * x_61);
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
