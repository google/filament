struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct buf1 {
  v1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_8 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m24 : mat2x2<f32>;
  var a : f32;
  var v2 : vec2<f32>;
  var v3 : vec2<f32>;
  let x_40 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_42 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_44 : f32 = x_8.v1.x;
  let x_47 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  m24 = mat2x2<f32>(vec2<f32>(x_40, x_42), vec2<f32>((x_44 * 1.0), x_47));
  let x_51 : mat2x2<f32> = m24;
  a = x_51[0u].x;
  v2 = vec2<f32>(1.0, 1.0);
  let x_53 : vec2<f32> = v2;
  let x_54 : f32 = a;
  let x_55 : vec2<f32> = vec2<f32>(x_54, 1.0);
  v3 = reflect(x_53, x_55);
  let x_58 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_59 : vec2<f32> = v3;
  let x_61 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  x_GLF_color = vec4<f32>(x_58, x_59.x, x_59.y, x_61);
  let x_66 : f32 = x_8.v1.y;
  let x_68 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_66 == x_68)) {
    let x_73 : vec4<f32> = x_GLF_color;
    x_GLF_color = vec4<f32>(x_73.x, vec2<f32>(0.0, 0.0).x, vec2<f32>(0.0, 0.0).y, x_73.w);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
