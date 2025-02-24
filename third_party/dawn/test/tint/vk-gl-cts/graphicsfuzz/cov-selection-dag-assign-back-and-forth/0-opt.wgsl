struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf0;

fn main_1() {
  var v : vec4<f32>;
  let x_25 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  let x_28 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_31 : i32 = x_5.x_GLF_uniform_int_values[1].el;
  let x_34 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4<f32>(f32(x_25), f32(x_28), f32(x_31), f32(x_34));
  let x_37 : vec4<f32> = x_GLF_color;
  v = x_37;
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  let x_38 : vec4<f32> = v;
  x_GLF_color = x_38;
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
