struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 5u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : f32;
  var b : f32;
  var x_51 : bool;
  var x_52_phi : bool;
  let x_28 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  let x_30 : f32 = x_6.x_GLF_uniform_float_values[3].el;
  let x_32 : f32 = x_6.x_GLF_uniform_float_values[3].el;
  let x_34 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  a = select(x_28, x_30, (x_32 > x_34));
  let x_37 : f32 = a;
  b = cos(log(x_37));
  let x_40 : f32 = b;
  x_GLF_color = vec4<f32>(x_40, x_40, x_40, x_40);
  let x_42 : f32 = b;
  let x_44 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_45 : bool = (x_42 > x_44);
  x_52_phi = x_45;
  if (x_45) {
    let x_48 : f32 = b;
    let x_50 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    x_51 = (x_48 < x_50);
    x_52_phi = x_51;
  }
  let x_52 : bool = x_52_phi;
  if (x_52) {
    let x_56 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_58 : f32 = x_6.x_GLF_uniform_float_values[4].el;
    let x_60 : f32 = x_6.x_GLF_uniform_float_values[4].el;
    let x_62 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    x_GLF_color = vec4<f32>(x_56, x_58, x_60, x_62);
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
