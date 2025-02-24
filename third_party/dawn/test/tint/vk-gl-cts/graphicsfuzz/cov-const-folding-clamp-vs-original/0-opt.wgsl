struct buf0 {
  one : f32,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn main_1() {
  var f : f32;
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  let x_23 : f32 = x_6.one;
  f = clamp(x_23, 1.0, 1.0);
  let x_25 : f32 = f;
  let x_27 : f32 = x_6.one;
  if ((x_25 > x_27)) {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  } else {
    let x_32 : f32 = f;
    x_GLF_color = vec4<f32>(x_32, 0.0, 0.0, 1.0);
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
