struct buf0 {
  one : i32,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn main_1() {
  var x_24 : vec4<f32>;
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  let x_26 : i32 = x_6.one;
  if ((x_26 == 0)) {
    return;
  }
  x_24 = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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

fn func_() -> vec4<f32> {
  return vec4<f32>(1.0, 0.0, 0.0, 1.0);
}
