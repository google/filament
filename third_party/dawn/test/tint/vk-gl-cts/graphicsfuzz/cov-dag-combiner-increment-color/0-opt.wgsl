struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 4u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : f32;
  let x_30 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  a = x_30;
  let x_32 : f32 = x_6.x_GLF_uniform_float_values[3].el;
  let x_34 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_32 > x_34)) {
    let x_39 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_40 : f32 = a;
    a = (x_40 + x_39);
    let x_42 : f32 = a;
    x_GLF_color = vec4<f32>(x_42, x_42, x_42, x_42);
    let x_45 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_47 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    if ((x_45 > x_47)) {
      let x_52 : f32 = x_GLF_color.x;
      let x_53 : f32 = a;
      a = (x_53 + x_52);
      let x_56 : f32 = x_6.x_GLF_uniform_float_values[2].el;
      x_GLF_color = vec4<f32>(x_56, x_56, x_56, x_56);
    }
  }
  let x_58 : f32 = a;
  x_GLF_color = vec4<f32>(x_58, 0.0, 0.0, 1.0);
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
