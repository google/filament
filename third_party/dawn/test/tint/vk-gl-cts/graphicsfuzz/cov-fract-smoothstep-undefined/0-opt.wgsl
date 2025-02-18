struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v1 : vec2<f32>;
  var b : vec2<f32>;
  var a : f32;
  var x_51 : bool;
  var x_52_phi : bool;
  let x_30 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  v1 = vec2<f32>(x_30, x_30);
  let x_32 : vec2<f32> = v1;
  b = fract(x_32);
  let x_34 : vec2<f32> = b;
  a = smoothstep(vec2<f32>(1.0, 1.0), vec2<f32>(2.0, 2.0), x_34).x;
  let x_38 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_39 : f32 = a;
  let x_40 : f32 = a;
  let x_42 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  x_GLF_color = vec4<f32>(x_38, x_39, x_40, x_42);
  let x_45 : f32 = b.x;
  let x_46 : bool = (x_45 < 1.0);
  x_52_phi = x_46;
  if (x_46) {
    let x_50 : f32 = b.y;
    x_51 = (x_50 < 1.0);
    x_52_phi = x_51;
  }
  let x_52 : bool = x_52_phi;
  if (x_52) {
    let x_57 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_59 : f32 = b.x;
    let x_61 : f32 = b.y;
    let x_63 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4<f32>(x_57, x_59, x_61, x_63);
  } else {
    let x_66 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4<f32>(x_66, x_66, x_66, x_66);
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
